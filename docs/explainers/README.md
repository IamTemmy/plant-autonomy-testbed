# Explainers

Plain-language walkthroughs of what each part of the project actually does, written so a non-technical reader can follow along. These are *not* technical reference (see [`docs/decision-log.md`](../decision-log.md) for design rationale and [`hub/`](../../hub/) and [`firmware/`](../../firmware/) for the actual code and configuration).

Each explainer is scoped to one section of the project. They use whatever metaphor fits that section best — the metaphors are not consistent across files, and that is intentional. The hub is an office building; firmware might end up as something else entirely.

## What's here

- [`phase3-hub.md`](phase3-hub.md) — the office-building analogy for the Pi-side services: broker, listener, database, systemd, credentials. Covers everything in `hub/01-pi-setup/` through `hub/05-listener-service/`.

## What's not here yet

Other explainers will appear as their corresponding parts of the project are built. Likely upcoming:

- A Phase 1 walkthrough of the bench-validation work (BME280, pump, soil moisture, etc.)
- A Phase 2 walkthrough of the WROVER's firmware once it exists
- A dashboard walkthrough once the Streamlit app exists
