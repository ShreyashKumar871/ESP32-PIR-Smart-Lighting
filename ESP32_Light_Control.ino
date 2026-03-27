// ESP32 - 1 PIR Sensor → 2 Relays + Web Control Dashboard
// PIR auto-trigger is ACTIVE only between 18:00–06:45 IST
// Manual web control is unrestricted (24/7)
// OVERRIDE button forces both relays ON 24/7 (no time gate)

#include <WiFi.h>
#include <WebServer.h>
#include <time.h>

// ===== WIFI CREDENTIALS - CHANGE THESE =====
const char* ssid     = "YourNetworkName";
const char* password = "YourPassword123";
// ============================================

// NTP settings — IST = UTC+5:30
const char* ntpServer   = "pool.ntp.org";
const long  gmtOffset   = 19800;   // 5h 30m in seconds
const int   daylightOff = 0;       // India has no DST

// Pin Definitions
#define PIR1_PIN     14
#define RELAY1_PIN   33
#define RELAY2_PIN   32

#define RELAY_ON_DURATION 5000  // 5 seconds

unsigned long pirLastTriggered = 0;

bool relay1ManualOn = false;
bool relay2ManualOn = false;
bool overrideActive = false;

bool relay1State = false;
bool relay2State = false;

WebServer server(80);

// ─── Helper Functions ─────────────────────────────────────────────────────────
void setRelay(int pin, bool* state, bool on) {
  digitalWrite(pin, on ? LOW : HIGH);
  *state = on;
}

// Returns true if current IST time is within 18:00–06:45
bool isWithinPirWindow() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return true; // fail-safe: allow if NTP down
  int h = timeinfo.tm_hour;
  int m = timeinfo.tm_min;
  if (h >= 18) return true;
  if (h < 6)   return true;
  if (h == 6 && m <= 45) return true;
  return false;
}

// ─── Web Page HTML ────────────────────────────────────────────────────────────
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 Relay Control</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Rajdhani:wght@400;600;700&display=swap');
  :root {
    --bg: #0a0e1a;
    --panel: #111827;
    --border: #1e293b;
    --accent: #00d4ff;
    --accent2: #ff6b35;
    --on: #00ff88;
    --off: #334155;
    --text: #e2e8f0;
    --muted: #64748b;
    --override: #f59e0b;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    background: var(--bg);
    color: var(--text);
    font-family: 'Rajdhani', sans-serif;
    min-height: 100vh;
    padding: 24px 16px;
    background-image: radial-gradient(ellipse at 20% 50%, rgba(0,212,255,0.04) 0%, transparent 60%),
                      radial-gradient(ellipse at 80% 20%, rgba(255,107,53,0.04) 0%, transparent 60%);
  }
  header { text-align: center; margin-bottom: 28px; }
  header h1 {
    font-size: 2rem; font-weight: 700; letter-spacing: 4px;
    text-transform: uppercase; color: var(--accent);
    text-shadow: 0 0 20px rgba(0,212,255,0.4);
  }
  header p {
    font-family: 'Share Tech Mono', monospace;
    color: var(--muted); font-size: 0.75rem;
    margin-top: 6px; letter-spacing: 2px;
  }
  .main-panel { max-width: 420px; margin: 0 auto 20px; }
  .pir-group {
    background: var(--panel); border: 1px solid var(--border);
    border-radius: 12px; padding: 20px;
    position: relative; overflow: hidden;
  }
  .pir-group::before {
    content: ''; position: absolute; top: 0; left: 0; right: 0; height: 2px;
    background: linear-gradient(90deg, transparent, var(--accent), transparent);
  }
  .pir-label {
    font-family: 'Share Tech Mono', monospace; font-size: 0.65rem;
    letter-spacing: 3px; color: var(--muted); text-transform: uppercase; margin-bottom: 10px;
  }
  .pir-row { display: flex; gap: 16px; margin-bottom: 18px; }
  .pir-status { display: flex; align-items: center; gap: 8px; }
  .pir-dot { width: 8px; height: 8px; border-radius: 50%; background: var(--off); transition: all 0.3s; }
  .pir-dot.active { background: var(--on); box-shadow: 0 0 8px var(--on); }
  .pir-name { font-size: 0.95rem; font-weight: 700; letter-spacing: 1px; }
  .relay-card {
    background: rgba(255,255,255,0.03); border: 1px solid var(--border);
    border-radius: 8px; padding: 14px; margin-bottom: 10px;
    display: flex; align-items: center; justify-content: space-between; transition: border-color 0.3s;
  }
  .relay-card.on { border-color: var(--on); background: rgba(0,255,136,0.05); }
  .relay-info { display: flex; align-items: center; gap: 10px; }
  .relay-indicator {
    width: 10px; height: 10px; border-radius: 50%;
    background: var(--off); transition: all 0.3s; flex-shrink: 0;
  }
  .relay-indicator.on { background: var(--on); box-shadow: 0 0 10px var(--on); }
  .relay-name { font-weight: 600; font-size: 0.95rem; }
  .relay-state { font-family: 'Share Tech Mono', monospace; font-size: 0.7rem; color: var(--muted); margin-top: 2px; }
  .relay-state.on { color: var(--on); }
  .btn-group { display: flex; gap: 6px; }
  button {
    font-family: 'Rajdhani', sans-serif; font-weight: 700; font-size: 0.75rem;
    letter-spacing: 1px; padding: 5px 10px; border-radius: 5px;
    border: 1px solid; cursor: pointer; text-transform: uppercase; transition: all 0.2s;
  }
  .btn-on { background: transparent; border-color: var(--on); color: var(--on); }
  .btn-on:hover { background: var(--on); color: #000; }
  .btn-off { background: transparent; border-color: var(--accent2); color: var(--accent2); }
  .btn-off:hover { background: var(--accent2); color: #fff; }
  .master-panel { max-width: 420px; margin: 0 auto 12px; display: grid; grid-template-columns: 1fr 1fr; gap: 12px; }
  .master-btn { padding: 12px; font-size: 0.85rem; border-radius: 8px; letter-spacing: 2px; }
  .master-on { border-color: var(--on); color: var(--on); background: transparent; }
  .master-on:hover { background: var(--on); color: #000; }
  .master-off { border-color: var(--accent2); color: var(--accent2); background: transparent; }
  .master-off:hover { background: var(--accent2); color: #fff; }

  /* ── OVERRIDE PANEL ───────────────────────── */
  .override-panel {
    max-width: 420px; margin: 0 auto 20px;
    background: var(--panel); border: 1px solid var(--border);
    border-radius: 12px; padding: 16px 20px;
    position: relative; overflow: hidden; transition: border-color 0.4s;
  }
  .override-panel.active {
    border-color: var(--override);
    background: rgba(245,158,11,0.06);
    box-shadow: 0 0 18px rgba(245,158,11,0.15);
  }
  .override-panel::before {
    content: ''; position: absolute; top: 0; left: 0; right: 0; height: 2px;
    background: linear-gradient(90deg, transparent, var(--override), transparent);
  }
  .override-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: 10px; }
  .override-title { font-weight: 700; font-size: 0.9rem; letter-spacing: 2px; text-transform: uppercase; }
  .override-badge {
    font-family: 'Share Tech Mono', monospace; font-size: 0.6rem;
    padding: 3px 8px; border-radius: 4px; letter-spacing: 1px;
    background: rgba(100,116,139,0.2); color: var(--muted); font-weight: 700;
  }
  .override-panel.active .override-badge {
    background: rgba(245,158,11,0.25); color: var(--override);
  }
  .override-desc {
    font-size: 0.75rem; line-height: 1.4; color: var(--muted); margin-bottom: 12px;
  }
  .override-actions { display: flex; gap: 8px; }
  .override-actions button { flex: 1; padding: 8px; }
  
  /* ── TIME BANNER ─────────────────────────────── */
  .time-banner {
    max-width: 420px; margin: 0 auto 16px;
    background: rgba(0,255,136,0.08); border: 1px solid rgba(0,255,136,0.25);
    border-radius: 8px; padding: 8px 12px; text-align: center;
    font-family: 'Share Tech Mono', monospace; font-size: 0.7rem;
    letter-spacing: 1px; color: var(--on); font-weight: 600;
  }
  .time-banner.inactive {
    background: rgba(255,107,53,0.08); border-color: rgba(255,107,53,0.25);
    color: var(--accent2);
  }

  footer {
    max-width: 420px; margin: 24px auto 0;
    text-align: center; font-family: 'Share Tech Mono', monospace;
    font-size: 0.65rem; color: var(--muted); letter-spacing: 1px;
  }
  footer span { color: var(--accent); }
</style>
</head>
<body>
<header>
  <h1>Relay Control</h1>
  <p>ESP32 Web Dashboard</p>
</header>

<div class="time-banner" id="time-banner">🟢 PIR AUTO-TRIGGER ACTIVE · --:-- IST</div>

<div class="override-panel" id="override-panel">
  <div class="override-header">
    <div class="override-title">⚡ OVERRIDE MODE</div>
    <div class="override-badge" id="override-badge">INACTIVE</div>
  </div>
  <div class="override-desc" id="override-desc">
    Forces both relays ON at all times — ignores PIR schedule & time gate. Active 24/7 until manually disabled.
  </div>
  <div class="override-actions">
    <button class="btn-on master-on" id="btn-ov-on" onclick="setOverride(1)">ENABLE</button>
    <button class="btn-off master-off" id="btn-ov-off" style="display:none;" onclick="setOverride(0)">DISABLE</button>
  </div>
</div>

<div class="master-panel">
  <button class="master-btn master-on" onclick="controlAll('on')">ALL ON</button>
  <button class="master-btn master-off" onclick="controlAll('off')">ALL OFF</button>
</div>

<div class="main-panel">
  <div class="pir-group">
    <div class="pir-label">Motion Sensor</div>
    <div class="pir-row">
      <div class="pir-status">
        <div class="pir-dot" id="pir1-dot"></div>
        <div class="pir-name">PIR 1</div>
      </div>
    </div>

    <div class="relay-card" id="card-r1">
      <div class="relay-info">
        <div class="relay-indicator" id="ind-r1"></div>
        <div>
          <div class="relay-name">Relay 1</div>
          <div class="relay-state" id="state-r1">STANDBY</div>
        </div>
      </div>
      <div class="btn-group">
        <button class="btn-on" onclick="control('relay1','on')">ON</button>
        <button class="btn-off" onclick="control('relay1','off')">OFF</button>
      </div>
    </div>

    <div class="relay-card" id="card-r2">
      <div class="relay-info">
        <div class="relay-indicator" id="ind-r2"></div>
        <div>
          <div class="relay-name">Relay 2</div>
          <div class="relay-state" id="state-r2">STANDBY</div>
        </div>
      </div>
      <div class="btn-group">
        <button class="btn-on" onclick="control('relay2','on')">ON</button>
        <button class="btn-off" onclick="control('relay2','off')">OFF</button>
      </div>
    </div>
  </div>
</div>

<footer>
  <span id="ip-display">---.---.---.---</span>
</footer>

<script>
  function control(relay, action) {
    fetch('/control?relay=' + relay + '&action=' + action);
  }
  function controlAll(action) {
    fetch('/control?relay=all&action=' + action);
  }
  function setOverride(state) {
    fetch('/override?state=' + state);
  }
  async function fetchStatus() {
    try {
      const res = await fetch('/status');
      const d = await res.json();
      document.getElementById('ip-display').textContent = d.ip;
      updateRelay(1, d.relay1);
      updateRelay(2, d.relay2);
      document.getElementById('pir1-dot').className = 'pir-dot' + (d.pir1 ? ' active' : '');

      // Time banner
      const banner = document.getElementById('time-banner');
      if (d.pirWindow) {
        banner.textContent = '🟢 PIR AUTO-TRIGGER ACTIVE · ' + d.time + ' IST';
        banner.className = 'time-banner';
      } else {
        banner.textContent = '🔴 PIR INACTIVE (06:45–18:00) · ' + d.time + ' IST';
        banner.className = 'time-banner inactive';
      }

      // Override panel
      const panel  = document.getElementById('override-panel');
      const badge  = document.getElementById('override-badge');
      const desc   = document.getElementById('override-desc');
      const btnOn  = document.getElementById('btn-ov-on');
      const btnOff = document.getElementById('btn-ov-off');
      if (d.override) {
        panel.classList.add('active');
        badge.textContent = 'ACTIVE';
        desc.textContent  = '⚡ Both relays forced ON — time gate bypassed. Disable to restore normal operation.';
        btnOn.style.display  = 'none';
        btnOff.style.display = 'block';
      } else {
        panel.classList.remove('active');
        badge.textContent = 'INACTIVE';
        desc.textContent  = 'Forces both relays ON at all times — ignores PIR schedule & time gate. Active 24/7 until manually disabled.';
        btnOn.style.display  = 'block';
        btnOff.style.display = 'none';
      }
    } catch(e) {}
  }
  function updateRelay(num, isOn) {
    const card  = document.getElementById('card-r'  + num);
    const ind   = document.getElementById('ind-r'   + num);
    const state = document.getElementById('state-r' + num);
    card.className  = 'relay-card'      + (isOn ? ' on' : '');
    ind.className   = 'relay-indicator' + (isOn ? ' on' : '');
    state.className = 'relay-state'     + (isOn ? ' on' : '');
    state.textContent = isOn ? 'ACTIVE' : 'STANDBY';
  }
  setInterval(fetchStatus, 1000);
  fetchStatus();
</script>
</body>
</html>
)rawliteral";

// ─── HTTP Handlers ────────────────────────────────────────────────────────────
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleControl() {
  String relay  = server.arg("relay");
  String action = server.arg("action");
  bool on = (action == "on");

  if      (relay == "relay1") { relay1ManualOn = on; setRelay(RELAY1_PIN, &relay1State, on); }
  else if (relay == "relay2") { relay2ManualOn = on; setRelay(RELAY2_PIN, &relay2State, on); }
  else if (relay == "all") {
    relay1ManualOn = relay2ManualOn = on;
    setRelay(RELAY1_PIN, &relay1State, on);
    setRelay(RELAY2_PIN, &relay2State, on);
  }
  server.send(200, "text/plain", "OK");
}

void handleOverride() {
  String state = server.arg("state");
  overrideActive = (state == "1");

  if (overrideActive) {
    setRelay(RELAY1_PIN, &relay1State, true);
    setRelay(RELAY2_PIN, &relay2State, true);
    Serial.println("Override ENABLED → Both relays forced ON (24/7)");
  } else {
    if (!relay1ManualOn) setRelay(RELAY1_PIN, &relay1State, false);
    if (!relay2ManualOn) setRelay(RELAY2_PIN, &relay2State, false);
    Serial.println("Override DISABLED → Normal operation resumed");
  }
  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  struct tm timeinfo;
  bool ntpOk = getLocalTime(&timeinfo);
  char timeBuf[6] = "--:--";
  if (ntpOk) strftime(timeBuf, sizeof(timeBuf), "%H:%M", &timeinfo);

  bool inWindow = isWithinPirWindow();
  String ipStr  = WiFi.localIP().toString();

  char json[260];
  snprintf(json, sizeof(json),
    "{\"relay1\":%s,\"relay2\":%s,"
    "\"pir1\":%s,"
    "\"override\":%s,\"pirWindow\":%s,"
    "\"time\":\"%s\",\"ip\":\"%s\"}",
    relay1State    ? "true" : "false",
    relay2State    ? "true" : "false",
    digitalRead(PIR1_PIN) ? "true" : "false",
    overrideActive ? "true" : "false",
    inWindow       ? "true" : "false",
    timeBuf,
    ipStr.c_str()
  );

  server.send(200, "application/json", json);
}

// ─── Setup ────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  pinMode(PIR1_PIN, INPUT);
  pinMode(RELAY1_PIN, OUTPUT); digitalWrite(RELAY1_PIN, HIGH);
  pinMode(RELAY2_PIN, OUTPUT); digitalWrite(RELAY2_PIN, HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 20) {
      Serial.println("\nFailed! Check: 1) SSID/password  2) 2.4GHz band  3) Router firewall");
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected! Open this IP in your browser: " + WiFi.localIP().toString());

    configTime(gmtOffset, daylightOff, ntpServer);
    Serial.print("Syncing NTP time");
    struct tm timeinfo;
    int ntpAttempts = 0;
    while (!getLocalTime(&timeinfo) && ntpAttempts < 10) {
      delay(500); Serial.print("."); ntpAttempts++;
    }
    if (getLocalTime(&timeinfo)) {
      char buf[32];
      strftime(buf, sizeof(buf), "%H:%M:%S IST", &timeinfo);
      Serial.println("\nTime synced: " + String(buf));
    } else {
      Serial.println("\nNTP sync failed — clock display will show --:--");
    }

    server.enableCORS(true);
    server.on("/",         handleRoot);
    server.on("/control",  handleControl);
    server.on("/override", handleOverride);
    server.on("/status",   handleStatus);
    server.begin();
    Serial.println("Web server started.");
  } else {
    Serial.println("Running without WiFi — PIR/relay logic still active.");
  }

  delay(2000); // PIR calibration
}

// ─── Loop ─────────────────────────────────────────────────────────────────────
void loop() {
  if (WiFi.status() == WL_CONNECTED) server.handleClient();

  unsigned long currentTime = millis();

  // ── Override: always keep relays ON, skip all other logic ──
  if (overrideActive) {
    if (!relay1State) setRelay(RELAY1_PIN, &relay1State, true);
    if (!relay2State) setRelay(RELAY2_PIN, &relay2State, true);
    delay(100);
    return;
  }

  // ── PIR logic: only runs between 18:00–06:45 IST ──
  if (isWithinPirWindow()) {
    if (digitalRead(PIR1_PIN) == HIGH) {
      pirLastTriggered = currentTime;
      if (!relay1ManualOn) setRelay(RELAY1_PIN, &relay1State, true);
      if (!relay2ManualOn) setRelay(RELAY2_PIN, &relay2State, true);
      Serial.println("PIR triggered (in window) → Relay 1 & 2 ON");
    }

    // Auto OFF after RELAY_ON_DURATION
    if (pirLastTriggered != 0 &&
        currentTime - pirLastTriggered >= RELAY_ON_DURATION) {
      if (!relay1ManualOn) setRelay(RELAY1_PIN, &relay1State, false);
      if (!relay2ManualOn) setRelay(RELAY2_PIN, &relay2State, false);
    }
  } else {
    // Outside window — turn off PIR-triggered relays (manual overrides remain)
    if (!relay1ManualOn && relay1State) setRelay(RELAY1_PIN, &relay1State, false);
    if (!relay2ManualOn && relay2State) setRelay(RELAY2_PIN, &relay2State, false);
    pirLastTriggered = 0;
  }

  delay(100);
}
