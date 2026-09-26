#!/usr/bin/env python3
"""Inject the platform specs from paper/PLATFORM_SPECS.md into paper/main.tex,
replacing the 7 remaining \\authorcheck{...} placeholders.

Run after filling every TODO in PLATFORM_SPECS.md:
    python tools/fill_specs.py

It refuses to run while any TODO remains, and reports how many placeholders
are left in main.tex afterward.

SPDX-License-Identifier: Apache-2.0
"""
import re, sys, os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SPECS = os.path.join(ROOT, "paper", "PLATFORM_SPECS.md")
TEX = os.path.join(ROOT, "paper", "main.tex")


def load_specs():
    vals = {}
    for line in open(SPECS, encoding="utf-8"):
        m = re.match(r"^([A-Z_]+):\s*(.+?)\s*(?:#.*)?$", line)
        if m:
            vals[m.group(1)] = m.group(2).strip()
    return vals


def main():
    v = load_specs()
    todo = [k for k, val in v.items() if val == "TODO"]
    if todo:
        sys.exit("Fill these keys in paper/PLATFORM_SPECS.md first: "
                 + ", ".join(todo))

    # Each entry: the exact \authorcheck{...} inner text -> replacement phrase
    # that reads grammatically after the preceding words in main.tex.
    repl = {
        "name RTOS or bare-metal environment, sleep mode, and wake sources":
            f"(the {v['SLEEP_MODE']} mode of the {v['MCU_ENV']} environment), "
            f"entered from the firmware event loop and left on {v['WAKE_SOURCES']}",
        "state what was run on the RISC-V board: firmware build, functional test, or trace replay":
            f", where we ran {v['RISCV_CHECK']}",
        "SoC model, Android and kernel versions":
            f"({v['PHONE_SOC']}, Android {v['PHONE_ANDROID']}, kernel {v['PHONE_KERNEL']})",
        "SoC model, OS version":
            f"({v['TABLET_SOC']}, {v['TABLET_OS']})",
        "MCU model, radio":
            f"({v['MCU_MODEL']}, {v['NODE_RADIO']})",
        "meter model and sampling rate":
            f"({v['METER_MODEL']} at {v['METER_RATE']})",
        "window length per workload":
            f"of {v['WINDOW_LENGTH']}",
    }

    tex = open(TEX, encoding="utf-8").read()
    applied = 0
    for inner, phrase in repl.items():
        token = "\\authorcheck{" + inner + "}"
        # a preceding space before the token often needs trimming for the
        # comma/paren replacements; handle " \authorcheck{...}" -> phrase.
        if (" " + token) in tex and phrase.startswith((",", ".", ")")):
            tex = tex.replace(" " + token, phrase)
            applied += 1
        elif token in tex:
            tex = tex.replace(token, phrase)
            applied += 1
        else:
            print(f"WARNING: placeholder not found: {inner[:40]}...")

    open(TEX, "w", encoding="utf-8").write(tex)
    remaining = tex.count("\\authorcheck{")
    print(f"Applied {applied} replacements. Remaining \\authorcheck: {remaining}")
    if remaining:
        print("(Some placeholders were not matched; check the WARNINGs above.)")
    else:
        print("All placeholders resolved. Recompile: cd paper && pdflatex main.tex")


if __name__ == "__main__":
    main()
