#!/usr/bin/env python3
"""Install (or update) the WiFi watchdog script on the Shelly via local RPC.

Runs on the Pi, which can reach the Shelly even under campus client isolation.
Idempotent: if a script of the same name already exists it is stopped and its
code replaced, so re-running just updates it.

Usage:
    python3 install_wifi_watchdog.py [path-to-wifi-watchdog.js]
Env:
    SHELLY_HOST   default 10.6.17.32
"""
import json
import os
import sys
import urllib.request

SHELLY_HOST = os.environ.get("SHELLY_HOST", "10.6.17.32")
SCRIPT_NAME = "wifi-watchdog"
JS_PATH = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "wifi-watchdog.js")


def rpc(method, params=None):
    body = json.dumps({"id": 1, "method": method, "params": params or {}}).encode()
    req = urllib.request.Request(
        "http://%s/rpc" % SHELLY_HOST, data=body,
        headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=10) as r:
        out = json.loads(r.read().decode())
    if "error" in out:
        raise RuntimeError("%s -> %s" % (method, out["error"]))
    return out.get("result", {})


def find_existing():
    for s in rpc("Script.List").get("scripts", []):
        if s.get("name") == SCRIPT_NAME:
            return s["id"]
    return None


def main():
    raw = open(JS_PATH, encoding="utf-8").read()
    # Shelly's Script.PutCode rejects multi-byte UTF-8 ("bad argument 'code'"),
    # so normalise to ASCII before upload (em-dash -> hyphen, drop the rest).
    code = raw.replace("\u2014", "-").encode("ascii", "ignore").decode()
    if code != raw:
        print("note: scrubbed non-ASCII from source before upload")
    print("Shelly:", SHELLY_HOST, "| script bytes:", len(code))

    sid = find_existing()
    if sid is not None:
        print("existing '%s' found at id %d; stopping + replacing" % (SCRIPT_NAME, sid))
        try:
            rpc("Script.Stop", {"id": sid})
        except RuntimeError:
            pass
    else:
        sid = rpc("Script.Create", {"name": SCRIPT_NAME})["id"]
        print("created script id", sid)

    # Upload code in chunks: first call overwrites, the rest append.
    # Gen4 PutCode caps code length per call well under 1 KB, so keep chunks small.
    CHUNK = 512
    first = True
    for i in range(0, len(code), CHUNK):
        rpc("Script.PutCode",
            {"id": sid, "code": code[i:i + CHUNK], "append": not first})
        first = False

    # Verify the full code length round-tripped onto the device.
    got = rpc("Script.GetCode", {"id": sid}).get("data", "")
    if len(got) != len(code):
        raise RuntimeError("uploaded length %d != source length %d"
                           % (len(got), len(code)))
    print("code uploaded and verified (%d bytes)" % len(got))

    rpc("Script.SetConfig", {"id": sid, "config": {"enable": True}})  # auto-start on boot
    rpc("Script.Start", {"id": sid})

    st = rpc("Script.GetStatus", {"id": sid})
    print("status: running=%s, enabled(on boot)=via SetConfig" % st.get("running"))
    if st.get("running"):
        print("OK — watchdog id %d is running and will auto-start on boot." % sid)
    else:
        print("WARNING — script not running; check Script.GetStatus / logs.")


if __name__ == "__main__":
    main()
