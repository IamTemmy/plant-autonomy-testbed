# 06 — Streamlit dashboard

Read-only web dashboard reading from the SQLite database populated by the
listener service (`hub/04-listener/`). Cream botanical theme with green primary
and semantic status colors. Mobile-friendly layout.

See [DL-037](../../docs/decision-log.md) for design rationale and the rationale
behind storing UTC in the database and displaying America/Chicago in the UI.

## Files

| File | Purpose |
|---|---|
| `dashboard.py` | Streamlit app; reads from `plant.db`, renders the dashboard |
| `.streamlit/config.toml` | Streamlit theme (light cream + green primary) |
| `README.md` | This file |

## What's shown

- **Header**: project title + overall health pill (green / amber / red)
- **Current state**: grow light state, real-time power, voltage, Shelly online status
- **Power draw**: interactive Plotly chart with 1-hour and 24-hour tabs
- **Recent activity**: last 10 actuator events
- **Active faults**: list of unacknowledged faults (banner shows clean status when none)
- **Listener run**: current run identifier and start time
- **Plant environment placeholders**: empty cards for the upcoming WROVER sensor data

## Visual references

Desktop view:
- ![Dashboard top](../../docs/images/05-dashboard-desktop-1.png)
- ![Dashboard middle](../../docs/images/05-dashboard-desktop-2.png)
- ![Dashboard bottom](../../docs/images/05-dashboard-desktop-3.png)

Mobile view (iPhone Safari):
- ![Dashboard mobile top](../../docs/images/06-dashboard-mobile-1.png)
- ![Dashboard mobile bottom](../../docs/images/06-dashboard-mobile-2.png)

## Setup procedure on the Pi

### 1. Install dependencies in the existing venv

```text
cd ~/plant-hub
source venv/bin/activate
pip install streamlit pandas streamlit-autorefresh plotly
```

### 2. Install the theme config

```text
mkdir -p ~/plant-hub/.streamlit
# Copy .streamlit/config.toml from this repo to ~/plant-hub/.streamlit/
```

### 3. Copy `dashboard.py` from this repo to `~/plant-hub/`

### 4. Run it manually

```text
cd ~/plant-hub
source venv/bin/activate
streamlit run dashboard.py --server.address 0.0.0.0 --server.port 8501
```

The `--server.address 0.0.0.0` flag binds to all interfaces so the dashboard
is reachable from other devices on the same LAN. Default is localhost-only.

### 5. View from another device on the LAN

```text
http://10.6.19.139:8501
```

The Pi's IP may differ if DHCP reassigns; the Shelly app or `ip addr` on the
Pi will show the current address.

## Timezone handling

The database stores timestamps in **UTC** (ISO 8601). The dashboard converts
all displayed timestamps to **America/Chicago** at render time via Python's
`zoneinfo.ZoneInfo("America/Chicago")`. The conversion is daylight-saving-aware,
so CDT and CST switch automatically with no code changes.

UTC in storage, local time in display is the standard convention for telemetry
systems. Rationale recorded in DL-037.

## What this layer does not yet handle

- **Not running as a systemd service.** Manual start only; next step.
- **Not yet remotely accessible.** LAN only. Tailscale comes in a future session.
- **No control surface.** Read-only by design for v1; remote actuation controls
  introduce safety and access-control concerns better deferred until the system
  is more mature.
- **No authentication.** LAN-accessible without credentials; acceptable given
  the same LAN already trusts the Pi over MQTT. Revisit when Tailscale lands.

## Future enhancements

- Auto-start at boot as a systemd service (`plant-dashboard.service`)
- Tailscale for remote access from outside JSU_DEVICE
- Dark theme as a toggle
- Sidebar navigation as more pages are added (events log, run history)
- Sensor panels populated as WROVER firmware comes online
