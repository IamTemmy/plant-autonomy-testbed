# Security model

This document records how the Plant Autonomy Testbed is secured, what its threat
model actually is, and — just as importantly — which common hardening measures were
**deliberately deferred** because they don't fit that model. The goal is to show the
security *reasoning*, not to accumulate controls for their own sake.

Every decision here is also traceable in the [decision log](docs/decision-log.md).

## What this system is

A single-operator, single-plant testbed: one ESP32-WROVER controller, one Raspberry
Pi hub, one grow-light plug, and one camera node, all on a private home network. It
physically actuates a water pump, so the controls that matter most are the ones that
prevent *unintended watering* — and those live in the firmware, not the network layer
(see the fail-safe and safety-tick work in the decision log, e.g. the independent pump
ceiling, sensor-freshness gating, reboot-safe transactions, and the maintenance pause).

## Network exposure

- **Nothing is exposed to the public internet.** The broker, the hub, the dashboard,
  and the devices are reachable only over the local network and a private
  [Tailscale](https://tailscale.com/) tailnet.
- **Remote access is via Tailscale**, limited to a small set of the operator's own
  enrolled devices. Reaching any service at all requires already being on the tailnet.
- The grow-light plug and camera are controlled over the LAN (HTTP RPC / local
  capture), not exposed externally.

This is the single most important fact about the threat model: **the set of parties
who can reach the broker or dashboard is exactly the operator's own devices.**

## Credentials

- MQTT uses authenticated connections (username + password); the broker does not
  accept anonymous clients.
- Hub-service credentials are held in an on-device `EnvironmentFile`
  (`/etc/plant-hub/credentials`) loaded by systemd, not committed to the repository.
- Firmware secrets (`secrets.h`) are git-ignored.
- No secrets appear in the decision log, dashboard, or telemetry.

## Deliberately deferred

These are standard measures that were evaluated and **intentionally not implemented**,
because for this deployment they would add operational fragility without closing a real
gap. They are documented here so the decision is explicit and revisitable.

### MQTT per-topic ACLs (deferred)

Splitting the single MQTT user into least-privilege roles (device / reader / operator,
so that e.g. only an "operator" credential may publish to `plant/cmd/#`) is the textbook
hardening step for a broker that actuates hardware.

It is **deferred** here because it defends against a party who has broker access but
should be limited — and by the network model above, *anyone who can reach the broker is
already the operator, on their own device.* The split would mean maintaining three
of one's own credentials across the firmware and every hub service (with a reflash and
coordinated `EnvironmentFile` changes), trading real fragility for protection against a
threat the deployment doesn't have.

**Revisit if:** the broker is ever exposed beyond the private tailnet, additional
untrusted clients share it, or the system grows to multiple plants/operators. At that
point the intended model is: `wrover` (publish telemetry, subscribe `plant/cmd/#`),
`hub` (subscribe-only, `plant/#`), `operator` (publish `plant/cmd/#`, read state).

### Camera-node authentication (deferred)

The camera node's capture path is read-only imagery on the LAN behind Tailscale. Adding
an auth layer is deferred on the same reasoning and is best folded into the vision-phase
build, when the capture/serving design is finalized rather than retrofitted.

## Reporting

This is a personal portfolio project with a single operator; there is no external
disclosure process. If you are reviewing the code and spot a security issue, please open
an issue on the repository.
