# Phase 3 — The Hub Building

A plain-language walkthrough of what the Pi-side of the project actually does. None of this is fiction — every "worker" described below is a real piece of software running on the Raspberry Pi right now.

## The setting

The Raspberry Pi sitting on the bench is, for our purposes, a tiny office building. It is plugged into the wall, connected to the WiFi, and runs 24 hours a day, seven days a week. Inside that building, a small staff of workers keeps the plant monitoring system functioning.

The building exists because something has to coordinate everything. The smart plug controlling the grow light needs to talk to something. Eventually, the sensors monitoring the basil plant will need to talk to something. And someone has to keep records of what's happening. That coordinator is the building.

## Electricity and phone lines

Before any work can happen, two things have to be in place: power and communications.

**The electricity** comes from the CanaKit wall adapter — a dedicated power supply specifically designed for the Pi. We chose it because cheaper chargers can deliver voltage that's slightly too low under load, which causes mysterious crashes that look like software problems. The CanaKit delivers reliably, the building stays on.

**The phone lines** are the JSU_DEVICE WiFi network. Everyone in the building uses the same WiFi to communicate. The smart plug down the hall is also on it. The developer's Mac uses it to connect in remotely when work needs to be done. Without WiFi, the building is isolated — no one can talk to anyone.

## The mail clerk

The first staff member to know about is the **mail clerk**. Technically he's called the MQTT broker, and he's the program named `mosquitto` running on the Pi.

His job is simple but essential: he routes messages. Anyone in the building (or in the next-door office) who wants to broadcast something — "the grow light just turned on," "current power draw is 15 watts," "I'm offline now" — sends the message to the mail clerk, and he distributes it to whoever asked to receive it.

The mail clerk doesn't store messages permanently. He doesn't interpret them. He's a router, not a librarian. A message arrives, he checks his list of who's interested, he forwards it to them. Then the message is gone.

He also enforces security: only people with the right credentials can send or receive mail through him. We set this up so that random devices on the same WiFi can't eavesdrop on the system.

## The first worker using the mail clerk

Across the hall is a worker we hired from outside — the **Shelly Plug US Gen4**. The Shelly is a smart power outlet that controls the grow light. Plug a lamp into the Shelly, plug the Shelly into the wall, and the Shelly can turn that lamp on and off based on instructions it receives over WiFi.

Every minute or so, the Shelly sends a postcard to the mail clerk reporting on its status: relay is off, voltage on the line is 117 volts, current draw is zero amps because the light isn't on, internal temperature is 35 degrees Celsius, cumulative energy used since installation is 129 watt-hours. The mail clerk forwards these postcards to anyone subscribed.

When someone in the building (or the developer, remotely) wants to turn the grow light on, they send a postcard to the mail clerk addressed to the Shelly. The mail clerk forwards it. The Shelly receives it, closes its relay, and the grow light comes on.

So the grow light is controllable from anywhere — via the building's mail clerk, via the Shelly's own phone app, via Apple HomeKit. All of these eventually arrive at the Shelly through different channels, but the relay closes the same way.

## The notebook

If nobody wrote anything down, the mail clerk's messages would vanish the moment they were delivered. To make any of this useful — to answer questions like *"how many times has the grow light come on this week"* or *"what's been the average voltage on this circuit over the last hour"* — somebody has to keep records.

That somebody writes in a large notebook with several distinct sections. The notebook is a SQLite database, and the sections are tables. Each section is for a different kind of record:

- **Raw archive** — every postcard the mail clerk ever forwarded, copied verbatim, in order, time-stamped. This section never gets edited. Even if we later realize we wrote down the wrong summary somewhere else, the raw archive is the source of truth.
- **Sensor readings** — measurements of things that change continuously: voltage, current, power, temperature, frequency, accumulated energy. Each reading is one row: when, from where, what was measured, the number, the unit.
- **Actuator events** — discrete state changes: grow light turned on, grow light turned off, water pump dispensed three seconds. Not the continuous numbers, just the moments when something happened.
- **System status** — which devices are online, which are offline, how long they've been running, how much memory is free. The health of the system itself, separate from the work it's doing.
- **Fault events** — when something goes wrong: low water, leak detected, watering didn't verify. Whether the operator has acknowledged each fault.
- **Runs** — the attendance log. Every time the secretary clocks in and out: when this work session started, when it ended, what kind of session it was (bench testing? deployment? calibration?). If the secretary crashes instead of clocking out properly, the "clocked out" field stays blank — an honest record of what went wrong.

## The secretary

The secretary is the worker who actually fills the notebook. Technically she's a Python program called the listener, and her job is straightforward: subscribe to every message the mail clerk has to offer, copy each one verbatim into the raw archive, and then write summaries into the appropriate other sections.

Her discipline is important. Every message gets archived raw before any summarizing happens. If she ever gets confused about how to summarize something — say, a new kind of postcard she's never seen before — she still archives it raw. We can always go back later and re-summarize from the raw archive. Nothing is lost.

She also distinguishes events from continuous readings carefully. When the Shelly's postcard mentions "the relay is currently off and the voltage is 117 volts," she only writes a new actuator-events row if the relay state actually changed since last time. The voltage reading goes to sensor-readings every time, since voltage is a continuous measurement that's expected to keep coming in. Without this discipline, the actuator-events section would be a thousand rows of "still off, still off, still off" and become useless for answering questions like "when did the light actually turn on."

## The building manager

The secretary used to only show up to work when somebody specifically called her in. Close the building for the night, and she went home. Reboot the building, and she didn't come back the next morning unless you re-hired her.

We fixed that by writing her job description for the **building manager**. The building manager is called `systemd`, and it's the same software that's already running every other automatic service on the Pi.

Now the building manager has a file describing the secretary's job: what she does, who she works for, when she's allowed to start (after the mail clerk has clocked in, and after the phone lines are up), and what to do if she ever has a problem and faints. The answer to that last one is: wait ten seconds, then call in a replacement secretary. Don't panic, don't loop on it forever, just calmly bring in someone else.

The building manager also knows to call her in automatically whenever the building starts up. So: power outage, Pi reboots, building manager wakes up, calls in mail clerk, then calls in secretary, and the system is back to running on its own. No human needed.

## The locked safe

The mail clerk requires credentials. The secretary, like any worker who needs to talk to him, has to know the password. But we didn't want the secretary to have to memorize the password — or worse, write it on a sticky note attached to her desk.

So we put the password in the building manager's locked safe. The safe is a small file at `/etc/plant-hub/credentials`, owned by the building's owner (root), readable only by the owner. When the building manager calls in the secretary in the morning, he reads the password out of the safe and hands it to her on the way to her desk. She uses it to authenticate with the mail clerk, never knows where it came from, and never writes it down.

This means rotating the password is straightforward: open the safe, write a new password, give the new password to the mail clerk and the Shelly so they accept it, restart the secretary so she picks up the new value. No password ever has to be typed into a terminal where it might get logged.

## What you can do today

The system is now real enough that you can interact with it from anywhere.

You can ask the mail clerk to turn the grow light on or off, and the Shelly will obey — the relay clicks, the bulb lights or goes dark.

You can ask the notebook how many times the grow light turned on this week, what voltage the Shelly is measuring right now, what the cumulative energy consumption has been since installation. You ask these questions in SQL (a query language); the answers come back as tidy tables of numbers.

The secretary is sitting at her desk filling pages, all day every day, whether or not anyone is asking her about her work. Hours pass, then days, and the notebook fills up with timestamped, structured data.

## What's also already built

A few more pieces of the building got added since the original walkthrough was written.

The **front-desk display board** (a Streamlit dashboard) reads the secretary's notebook and shows the current state on a cream-themed page with charts and status indicators. Anyone with a phone or a laptop can pull it up in a browser and see what's happening with the plant at a glance — current grow light state, recent power consumption, online status, the secretary's run history. It auto-refreshes every ten seconds, so the page is always within ten seconds of being current. Like the secretary, it has its own job description with the building manager and starts automatically whenever the building powers up.

The building also now has a **remote-access door**. Originally, you had to be inside the building (on the same WiFi as the Pi) to see the front-desk display. That's not great if you want to check on the plant while traveling. So we installed Tailscale, a kind of private connector that lets the operator's phone and laptop reach the building from anywhere — coffee shop, hotel, another country — without exposing the building to the public internet. Each authorized device gets a private address that only works for the operator's devices, all encrypted end to end.

Two devices currently have keys to the remote-access door: the operator's Mac and iPhone. Adding a new device requires explicit authorization through the Tailscale admin console. No random visitor can find or knock on the door.

## What's still to come

The building has space for more workers, and the mail clerk is ready to route their postcards to the secretary. Soon:

- A **sensor monitor** (the ESP32 WROVER firmware) will join the system from a different room. It will report on the basil plant's air temperature, humidity, light levels, soil moisture, and water reservoir level. Postcards to the mail clerk, captured by the secretary, written into the sensor-readings section. Once those postcards start arriving, the front-desk display's "Plant environment" panel — currently empty — will populate automatically.
- A **photographer** (the ESP32-CAM) will eventually take pictures of the plant on a schedule. This worker is delayed because the initial hardware didn't pass its job interview; we're getting replacement equipment.

The mail clerk and secretary already exist and don't need to change to accommodate any of these. The system is built so the staff can grow without rewriting the building.

## Where this all lives

The building (the Pi) is physical. You can touch it. It sits on the bench, plugged into the wall, blinking quietly.

The blueprints — every file we wrote, every decision we documented, every configuration the building manager uses — also live somewhere else, off-site, in a backup. That backup is the GitHub repository at `github.com/IamTemmy/plant-autonomy-testbed`. If the building burned down tomorrow, the blueprints would be safe; we could buy a new Pi, follow the procedures in the repo, and rebuild the building from scratch in an afternoon.

That's deliberate. The building is just the place where the work happens. The blueprints are the project.
