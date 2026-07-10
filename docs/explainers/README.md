# Explainers

Plain-language walkthroughs of what each part of the project actually does, written so a non-technical reader can follow along. These are *not* technical reference (see [`docs/decision-log.md`](../decision-log.md) for design rationale and [`hub/`](../../hub/) and [`firmware/`](../../firmware/) for the actual code and configuration).

Each explainer is scoped to one section of the project. They use whatever metaphor fits that section best — the metaphors are not consistent across files, and that is intentional. The hub is an office building; firmware might end up as something else entirely.

## What's here

- [`phase3-hub.md`](phase3-hub.md) — the office-building analogy for the Pi-side services: broker, listener, database, systemd, credentials. Covers everything in `hub/01-pi-setup/` through `hub/05-listener-service/`.

- [`tuning-from-data.md`](tuning-from-data.md) — how the project's thresholds (grow-light lux, the soil jump that flags external watering, the device-silence timeout, and more) were each read off the system's own logged history rather than guessed: the instrument-first, then-tune-from-the-distribution method.

## What's not here yet

Other explainers will appear as more of the project gets a plain-language write-up. The code and rationale for these already exist (in `firmware/`, `hub/`, and the decision log) — only the narrative explainer is still to come:

- A Phase 1 walkthrough of the bench-validation work (BME280, pump, soil moisture, etc.)
- A Phase 2 walkthrough of the WROVER's firmware
- A dashboard walkthrough
