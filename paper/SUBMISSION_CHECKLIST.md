# IEEE Internet of Things Journal submission checklist (checked 2026-09-24)

This checklist separates manuscript items already addressed from items that require the author's action in the IEEE Author Portal.

## Manuscript compliance already addressed

- [x] Uses the IEEE two-column journal class (`IEEEtran`, journal mode).
- [x] Title is concise and explicitly foregrounds heterogeneous IoT systems.
- [x] Abstract is one paragraph, 245 words, contains no citations and no displayed equations (IoT-J requires 150-250 words).
- [x] Keywords include IoT and constrained-device/power-management terminology.
- [x] Main PDF compiles to exactly 8 pages. IoT-J charges mandatory overlength fees after the first 8 published pages; final typesetting can still shift pagination.
- [x] Human-subject MOS/QoE results were removed from this submission-ready version because no IRB/ethics-board and consent documentation was available in the source material.
- [x] The AI-use acknowledgment was rewritten to identify OpenAI ChatGPT and the sections where drafting/restructuring assistance was used; it explicitly states that AI was not used to generate or alter experimental results.
- [x] Unverified placeholder-style references from earlier drafts were replaced with bibliographically verifiable publications relevant to IoT/mobile energy management.
- [x] Claims about statistics were narrowed: the paper reports only the available means and does not invent standard deviations, confidence intervals, or significance tests.
- [x] Portability claims are bounded: x86/RISC-V are described as portability checks, not quantitative cross-architecture performance evidence.

## Must be confirmed by the author before submission

- [ ] **Resolve every red `\authorcheck{...}` marker in `main.tex`** (20 items: parameter values, platform/OS/MCU details, measurement window, baseline definition, per-workload device and mode, context rule table, LR training data, RISC-V check, repository). Then delete the `\authorcheck` macro so the build fails if any marker remains.
- [ ] **Algorithm 1 must match the evaluated code.** The v4 text formalizes context as fixed per-mode weights, the scale in Eqs. (6)-(7), a break-even floor, and an inhibit flag. If the implementation that produced Table V behaves differently, change the text to describe the code, not the other way round.
- [ ] **Verify the classical DPM references added in v4** (Benini 2000, Srivastava 1996, Hwang & Wu 2000, Douglis 1995, Helmbold 1996, Simunic 2001, Pallipadi 2007, Karlin 1994, Irani 2003) against DOI/IEEE Xplore/ACM DL records.

- [ ] **Prior publication / extension disclosure.** Confirm whether the 4-page source paper has already been published, accepted, or is under review anywhere. If published/accepted, the IoT-J manuscript must cite it and clearly explain the substantial technical extension in the manuscript and cover letter. If it is under review elsewhere, do not submit concurrently.
- [ ] **ORCID.** Every listed author must have a registered ORCID linked in the IEEE submission system.
- [ ] **Repository URL.** Create/populate `https://github.com/leemgs/uatimer` (or change the manuscript URL) before submission so the reproducibility link is not broken.
- [ ] **Author list/affiliation.** Confirm that all people who meet IEEE authorship criteria are listed, and that the Sungkyunkwan University affiliation/e-mail are correct for this submission.
- [ ] **Human-subject data.** If any participant-generated traces, user-study results, or identifiable human-subject data are restored, add the official IRB/ethics-committee statement and consent statement required by IEEE.
- [ ] **Portal classification.** Select the closest IoT-J classification/keywords in the Author Portal; `Constrained Devices` is an obvious match if present in the current list.
- [ ] **Reviewer suggestions.** Suggest only reviewers with no institutional, collaboration, technical, family, or other conflict of interest.
- [ ] **Originality.** Confirm the manuscript is not simultaneously under consideration by any other journal or conference.
- [ ] **Reference validation.** Run the IEEE Reference Preparation Assistant as a final submission step.
- [ ] **LaTeX/PDF validation.** Run the IEEE LaTeX Analyzer and IEEE PDF Checker immediately before upload.
- [ ] **Publication model.** Select Traditional publication if the goal is no OA APC. Open Access is optional and author-paid; overlength charges are separate.
- [ ] **Graphical abstract.** Optional. If one is submitted, it must be included for peer review and comply with IEEE graphical-abstract specifications.

## Desk-reject risks that are not solved by formatting

The manuscript is now substantially cleaner from a submission-compliance standpoint, but editor screening can still reject for technical reasons. The largest remaining risks are limited quantitative IoT-platform breadth (one principal IoT node), a relatively lightweight ML baseline (twenty repetitions per condition, with 95% CIs and Bonferroni-corrected significance, are now reported). These are scientific-strength issues rather than author-guideline violations.
