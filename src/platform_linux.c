/*
 * platform_linux.c - Linux adaptation layer for UATimer.
 *
 * Detects events with epoll on one or more file descriptors (e.g. an
 * input device or an eventfd), arms/cancels a one-shot deadline with
 * timerfd, and maps suspend() to a caller-provided low-power action.
 * This mirrors the "epoll-based event hooks" integration described in
 * the manuscript. It contains no policy logic; it only drives the core.
 *
 * On Linux this file plus the policy core run entirely in user space and
 * require no kernel modification. Build with UA_HAVE_LINUX defined.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#define _POSIX_C_SOURCE 200809L
#include "uatimer.h"

#ifdef UA_HAVE_LINUX
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

typedef struct {
    int      epfd;
    int      tfd;             /* timerfd for one-shot deadlines */
    ua_ms_t (*do_suspend)(void *user); /* platform low-power action */
    void    *user;
} ua_linux_t;

static ua_ms_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ua_ms_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void lx_arm(void *ctx, ua_ms_t deadline_ms)
{
    ua_linux_t *l = (ua_linux_t *)ctx;
    ua_ms_t rel = deadline_ms - now_ms();
    if (rel < 1) rel = 1;
    struct itimerspec its;
    memset(&its, 0, sizeof(its));
    its.it_value.tv_sec  = rel / 1000;
    its.it_value.tv_nsec = (rel % 1000) * 1000000;
    timerfd_settime(l->tfd, 0, &its, NULL);
}

static void lx_cancel(void *ctx)
{
    ua_linux_t *l = (ua_linux_t *)ctx;
    struct itimerspec its;
    memset(&its, 0, sizeof(its));   /* zero disarms the timerfd */
    timerfd_settime(l->tfd, 0, &its, NULL);
}

static ua_ms_t lx_suspend(void *ctx)
{
    ua_linux_t *l = (ua_linux_t *)ctx;
    if (l->do_suspend) return l->do_suspend(l->user);
    return 0;
}

/* Create a Linux platform bound to the given policy. event_fds are
 * watched for readability; a read on any of them is treated as an event.
 * do_suspend performs the actual low-power transition (e.g. write to a
 * sysfs power state) and returns the slept duration in ms. */
int ua_linux_run(ua_timer_t *t, int *event_fds, int n_fds,
                 ua_ms_t (*do_suspend)(void *user), void *user,
                 volatile int *stop_flag)
{
    ua_linux_t l;
    memset(&l, 0, sizeof(l));
    l.do_suspend = do_suspend;
    l.user       = user;

    l.epfd = epoll_create1(0);
    l.tfd  = timerfd_create(CLOCK_MONOTONIC, 0);
    if (l.epfd < 0 || l.tfd < 0) return -1;

    struct epoll_event ev;
    for (int i = 0; i < n_fds; i++) {
        ev.events  = EPOLLIN;
        ev.data.fd = event_fds[i];
        epoll_ctl(l.epfd, EPOLL_CTL_ADD, event_fds[i], &ev);
    }
    ev.events  = EPOLLIN;
    ev.data.fd = l.tfd;
    epoll_ctl(l.epfd, EPOLL_CTL_ADD, l.tfd, &ev);

    ua_platform_t plat = { lx_suspend, lx_arm, lx_cancel, &l };
    ua_init(t, &t->cfg, now_ms());
    ua_on_event(t, &plat, now_ms());   /* arm initial threshold */

    struct epoll_event events[16];
    while (!(stop_flag && *stop_flag)) {
        int n = epoll_wait(l.epfd, events, 16, -1);
        if (n < 0) { if (errno == EINTR) continue; break; }
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            if (fd == l.tfd) {
                uint64_t exp;
                if (read(l.tfd, &exp, sizeof(exp)) != sizeof(exp)) { /* ignore */ }
                ua_on_expiry(t, &plat, now_ms(), /*has_pending=*/false);
            } else {
                char buf[256];
                while (read(fd, buf, sizeof(buf)) > 0) { /* drain */ }
                ua_on_event(t, &plat, now_ms());
            }
        }
    }

    close(l.tfd);
    close(l.epfd);
    return 0;
}
#endif /* UA_HAVE_LINUX */
