// wifi-watchdog.js - Shelly Plug US Gen4 on-device WiFi watchdog (DL-085).
//
// The Shelly has been observed to drop off WiFi and not auto-reconnect, going
// unreachable for hours while still powered (the Pi enforcer then logs
// "No route to host" every tick and the grow light is stuck in whatever state
// it was last left in). This is a documented Shelly firmware behaviour, not a
// signal problem (RSSI was healthy at the time).
//
// This script runs ON the device, independent of the network, and reboots the
// Shelly after a sustained WiFi loss so it rejoins fresh. After reboot, the
// switch returns to its prior state (initial_state = restore_last) and the
// Pi photoperiod enforcer re-asserts the correct state within ~2 min.
//
// It is a recovery of last resort, paired with a blind daily 02:00 reboot
// schedule (catches half-open states this check might miss). Conservative by
// design: only a *sustained* outage triggers a reboot, and the counter resets
// the instant WiFi returns, so brief blips and flaky-but-recovering links do
// not cause reboot loops.

let CHECK_PERIOD_S = 60;   // how often to check WiFi
let FAILS_TO_REBOOT = 5;   // consecutive failures before reboot (~5 min)

let fails = 0;

Timer.set(CHECK_PERIOD_S * 1000, true, function () {
  let wifi = Shelly.getComponentStatus("wifi");
  let connected = wifi && wifi.status === "got ip";

  if (connected) {
    if (fails > 0) {
      print("wifi-watchdog: reconnected, resetting counter (was ", fails, ")");
    }
    fails = 0;
    return;
  }

  fails = fails + 1;
  print("wifi-watchdog: wifi not connected (status=",
        wifi ? wifi.status : "unknown", "), fail ", fails, "/", FAILS_TO_REBOOT);

  if (fails >= FAILS_TO_REBOOT) {
    print("wifi-watchdog: sustained WiFi loss -> rebooting");
    Shelly.call("Shelly.Reboot", {});
  }
});

print("wifi-watchdog: started; checking every ", CHECK_PERIOD_S,
      "s, reboot after ", FAILS_TO_REBOOT, " consecutive failures");
