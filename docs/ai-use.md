# Use of AI in This Project

## Why this document

This project was built with the assistance of an AI system (Anthropic's Claude).
In the interest of transparency, this file records the *extent* of that
assistance — what the AI did, what I did, and how we worked together — so anyone
reading the repository has an honest picture of how it was made.

## What the AI was used for

- **Pair-design on the autonomous watering architecture** — working through the
  control-loop states, timing, thresholds, and failure handling.
- **Drafting firmware** — turning agreed designs into ESP32 / PlatformIO C++
  (e.g. the bottom-watering control loop), which I then built, flashed, and
  tested on real hardware.
- **A documentation-consistency audit** across the README, changelog, and
  decision log.
- **Authoring decision-log entries** from our working sessions.
- **Debugging support** — reasoning through sensor behaviour, calibration, and
  test results.

## What I did

- **All hardware and physical work**: wiring, sensor mounting and placement, the
  pump and reservoir plumbing, the sand topdressing, and every hands-on watering
  test.
- **Every real-world decision** and the overall direction of the project.
- **The judgment.** I approved every design before it was implemented and
  reviewed every code patch before applying it. I caught and corrected AI
  mistakes along the way — for example, timezone errors in log analysis, an
  incorrect dose figure, and image-format issues — and I made the final calls
  where the AI and I disagreed (for instance, setting the wet-calibration anchor
  at raw 2250 and the watering target at 85%, over the AI's more conservative
  suggestions).

## How we worked (the model)

The pattern was **consult-before-implement**: the AI proposed designs and I
decided; I directed and the AI executed. Nothing entered the repository
automatically — code was delivered as patches that I reviewed, applied,
committed, and pushed myself, after verifying it on the actual hardware. I
expected and relied on honest pushback rather than agreement, and disagreements
were resolved by my judgment. The AI was a partner working under human direction
and review, not an autonomous author.

## Traceability

This assistance is visible in the repository's own history rather than only
asserted here. The [decision log](decision-log.md), particularly from DL-102
onward, is timestamped and records the reasoning behind each change — including
where AI-assisted design and implementation happened — so the account is
auditable.

## My guideline

I treat AI as a design and implementation partner used under human direction,
review, and accountability — not as a substitute for my own engineering judgment
or the physical, real-world work the project depends on.
