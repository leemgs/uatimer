# UATimer reference implementation - build file.
# SPDX-License-Identifier: Apache-2.0

CC      ?= cc
CFLAGS  ?= -std=c11 -O2 -Wall -Wextra -Iinclude
LDLIBS  ?= -lm

# Enable the Linux epoll/timerfd backend when building on Linux.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
CFLAGS  += -DUA_HAVE_LINUX
endif

CORE    := src/uatimer.c src/lr_baseline.c
BUILD   := build

.PHONY: all test replay clean demo
all: $(BUILD)/uatimer_replay $(BUILD)/uatimer_test

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/uatimer_replay: $(CORE) src/replay.c src/platform_linux.c | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/uatimer_test: $(CORE) src/test_uatimer.c src/platform_linux.c | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

test: $(BUILD)/uatimer_test
	./$(BUILD)/uatimer_test

# Regenerate the example traces and replay them.
demo: $(BUILD)/uatimer_replay
	python3 tools/gen_trace.py
	@for w in web video sensing arvr; do \
	  echo "== $$w =="; \
	  ./$(BUILD)/uatimer_replay traces/$$w.trace config/mobile.cfg; \
	done

clean:
	rm -rf $(BUILD)
