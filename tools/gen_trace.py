#!/usr/bin/env python3
"""Generate synthetic inter-event traces for the UATimer replay harness.

These are illustrative workload shapes, NOT the datasets used for the
energy measurements in the manuscript. Each line is "<idle_ms> [mode]".
Modes: 0=energy, 1=balanced, 2=latency.

SPDX-License-Identifier: Apache-2.0
"""
import os
import random

random.seed(42)
OUT = os.path.join(os.path.dirname(__file__), "..", "traces")
os.makedirs(OUT, exist_ok=True)


def write(name, rows):
    path = os.path.join(OUT, f"{name}.trace")
    with open(path, "w") as f:
        f.write(f"# synthetic {name} trace (illustrative, not measured)\n")
        for r in rows:
            f.write(r + "\n")
    print("wrote", os.path.relpath(path))


def web(n=2000):
    # Bursts of quick touches, then long reading pauses. Balanced mode.
    rows = []
    while len(rows) < n:
        for _ in range(random.randint(3, 12)):          # burst
            rows.append(f"{random.randint(80, 400)} 1")
        rows.append(f"{random.randint(4000, 30000)} 1")  # reading pause
    return rows[:n]


def video(n=2000):
    # Near-continuous decode ticks with rare input. Balanced mode.
    return [f"{random.randint(30, 60)} 1" if random.random() > 0.05
            else f"{random.randint(2000, 8000)} 1" for _ in range(n)]


def sensing(n=2000):
    # Periodic sampling every ~10 s with jitter. Energy mode.
    return [f"{10000 + random.randint(-200, 200)} 0" for _ in range(n)]


def arvr(n=2000):
    # 60 Hz rendering events (~16.7 ms). Latency-critical mode.
    return [f"{16 + random.randint(0, 1)} 2" for _ in range(n)]


if __name__ == "__main__":
    write("web", web())
    write("video", video())
    write("sensing", sensing())
    write("arvr", arvr())
