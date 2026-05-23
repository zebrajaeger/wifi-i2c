#include <Arduino.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>

#include <algorithm>
#include <vector>

namespace {
constexpr char kApSsid[] = "wifi-i2c-setup";
constexpr char kPreferencesNamespace[] = "wifi";
constexpr char kSsidKey[] = "ssid";
constexpr char kPasswordKey[] = "password";
constexpr unsigned long kWifiConnectTimeoutMs = 15000;
const IPAddress kApIp(192, 168, 4, 1);
const IPAddress kApGateway(192, 168, 4, 1);
const IPAddress kApSubnet(255, 255, 255, 0);

struct WifiNetwork {
  String ssid;
  int32_t rssi;
  wifi_auth_mode_t encryptionType;
};

DNSServer dnsServer;
WebServer webServer(80);
bool provisioningActive = false;

String htmlEscape(const String &value) {
  String escaped;
  escaped.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i) {
    const char c = value[i];

    switch (c) {
      case '&':
        escaped += F("&amp;");
        break;
      case '<':
        escaped += F("&lt;");
        break;
      case '>':
        escaped += F("&gt;");
        break;
      case '"':
        escaped += F("&quot;");
        break;
      case '\'':
        escaped += F("&#39;");
        break;
      default:
        escaped += c;
        break;
    }
  }

  return escaped;
}

String authLabel(wifi_auth_mode_t encryptionType) {
  return encryptionType == WIFI_AUTH_OPEN ? F("open") : F("secured");
}

bool loadCredentials(String &ssid, String &password) {
  Preferences preferences;

  if (!preferences.begin(kPreferencesNamespace, true)) {
    Serial.println(F("[wifi] Failed to open preferences for reading."));
    return false;
  }

  ssid = preferences.getString(kSsidKey, "");
  password = preferences.getString(kPasswordKey, "");
  preferences.end();

  ssid.trim();
  Serial.print(F("[wifi] Stored SSID length: "));
  Serial.println(ssid.length());
  return ssid.length() > 0;
}

bool saveCredentials(const String &ssid, const String &password) {
  Preferences preferences;

  if (!preferences.begin(kPreferencesNamespace, false)) {
    Serial.println(F("[wifi] Failed to open preferences for writing."));
    return false;
  }

  const bool savedSsid = preferences.putString(kSsidKey, ssid) > 0;
  preferences.putString(kPasswordKey, password);
  preferences.end();

  Serial.print(F("[wifi] Credentials save result for SSID '"));
  Serial.print(ssid);
  Serial.print(F("': "));
  Serial.println(savedSsid ? F("ok") : F("failed"));
  return savedSsid;
}

bool connectToConfiguredWifi() {
  String ssid;
  String password;

  if (!loadCredentials(ssid, password)) {
    Serial.println(F("[wifi] No saved credentials found; starting provisioning."));
    return false;
  }

  Serial.print(F("[wifi] Connecting to SSID: "));
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  const unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - startedAt < kWifiConnectTimeoutMs) {
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("[wifi] Connected. IP address: "));
    Serial.println(WiFi.localIP());
    Serial.print(F("[wifi] RSSI: "));
    Serial.println(WiFi.RSSI());
    return true;
  }

  Serial.print(F("[wifi] Connection failed with status: "));
  Serial.println(WiFi.status());
  Serial.println(F("[wifi] Configured WiFi was not reachable; starting provisioning."));
  WiFi.disconnect(true);
  return false;
}

std::vector<WifiNetwork> scanWifiNetworks() {
  std::vector<WifiNetwork> networks;
  Serial.println(F("[scan] Starting WiFi scan."));
  const int networkCount = WiFi.scanNetworks();
  Serial.print(F("[scan] Raw networks found: "));
  Serial.println(networkCount);

  for (int i = 0; i < networkCount; ++i) {
    const String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) {
      continue;
    }

    const int32_t rssi = WiFi.RSSI(i);
    const wifi_auth_mode_t encryptionType = WiFi.encryptionType(i);
    auto existing = std::find_if(
        networks.begin(), networks.end(),
        [&ssid](const WifiNetwork &network) { return network.ssid == ssid; });

    if (existing == networks.end()) {
      networks.push_back({ssid, rssi, encryptionType});
    } else if (rssi > existing->rssi) {
      existing->rssi = rssi;
      existing->encryptionType = encryptionType;
    }
  }

  WiFi.scanDelete();
  std::sort(networks.begin(), networks.end(),
            [](const WifiNetwork &left, const WifiNetwork &right) {
              return left.rssi > right.rssi;
            });

  Serial.print(F("[scan] Unique visible SSIDs: "));
  Serial.println(networks.size());
  for (const WifiNetwork &network : networks) {
    Serial.print(F("[scan] "));
    Serial.print(network.rssi);
    Serial.print(F(" dBm "));
    Serial.println(network.ssid);
  }

  return networks;
}

String renderPage(const String &content) {
  String html;
  html.reserve(content.length() + 1200);
  html += F("<!doctype html><html lang=\"de\"><head><meta charset=\"utf-8\">");
  html += F("<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">");
  html += F("<title>WiFi Setup</title><style>");
  html += F("body{font-family:system-ui,-apple-system,BlinkMacSystemFont,Segoe UI,sans-serif;margin:0;background:#f6f7f9;color:#17202a}");
  html += F("main{max-width:520px;margin:0 auto;padding:28px 18px}");
  html += F("h1{font-size:1.6rem;margin:0 0 1rem}");
  html += F("label{display:block;font-weight:650;margin:.9rem 0 .35rem}");
  html += F("select,input,button{box-sizing:border-box;width:100%;font:inherit;padding:.72rem;border-radius:6px;border:1px solid #bdc7d3;background:white}");
  html += F("button{margin-top:1rem;border:0;background:#0f766e;color:white;font-weight:700;cursor:pointer}");
  html += F(".secondary{background:#344054}.message{padding:.8rem;border-radius:6px;background:#e6f4ea}.error{padding:.8rem;border-radius:6px;background:#fde7e9}");
  html += F(".meta{color:#667085;font-size:.9rem}.empty{padding:.8rem;border:1px dashed #bdc7d3;border-radius:6px;background:white}");
  html += F("</style></head><body><main>");
  html += content;
  html += F("</main></body></html>");
  return html;
}

String renderPortalContent(const String &message = "", const String &error = "") {
  const std::vector<WifiNetwork> networks = scanWifiNetworks();
  String content;

  content += F("<h1>WiFi Setup</h1>");

  if (message.length() > 0) {
    content += F("<p class=\"message\">");
    content += htmlEscape(message);
    content += F("</p>");
  }

  if (error.length() > 0) {
    content += F("<p class=\"error\">");
    content += htmlEscape(error);
    content += F("</p>");
  }

  content += F("<form method=\"post\" action=\"/save\">");
  content += F("<label for=\"ssid\">Netzwerk</label>");

  if (networks.empty()) {
    content += F("<div class=\"empty\">Keine WLANs gefunden. Seite neu laden, um erneut zu suchen.</div>");
  } else {
    content += F("<select id=\"ssid\" name=\"ssid\" required>");
    for (const WifiNetwork &network : networks) {
      content += F("<option value=\"");
      content += htmlEscape(network.ssid);
      content += F("\">");
      content += htmlEscape(network.ssid);
      content += F(" (");
      content += network.rssi;
      content += F(" dBm, ");
      content += authLabel(network.encryptionType);
      content += F(")</option>");
    }
    content += F("</select>");
  }

  content += F("<label for=\"password\">Passwort</label>");
  content += F("<input id=\"password\" name=\"password\" type=\"password\" autocomplete=\"current-password\">");
  content += F("<button type=\"submit\">Credentials speichern</button>");
  content += F("</form>");
  content += F("<form method=\"post\" action=\"/reboot\"><button class=\"secondary\" type=\"submit\">Controller neu starten</button></form>");
  content += F("<p class=\"meta\">Access Point: ");
  content += htmlEscape(kApSsid);
  content += F(" - Portal: http://");
  content += kApIp.toString();
  content += F("</p>");

  return content;
}

void sendPortal(const String &message = "", const String &error = "",
                int statusCode = 200) {
  webServer.send(statusCode, F("text/html"),
                 renderPage(renderPortalContent(message, error)));
}

void redirectToPortal() {
  Serial.print(F("[portal] Redirecting request for "));
  Serial.println(webServer.uri());
  webServer.sendHeader(F("Location"), String(F("http://")) + kApIp.toString(),
                       true);
  webServer.send(302, F("text/plain"), F(""));
}

void handleSaveCredentials() {
  String ssid = webServer.arg(F("ssid"));
  const String password = webServer.arg(F("password"));
  ssid.trim();

  if (ssid.length() == 0) {
    Serial.println(F("[portal] Rejected credential submission without SSID."));
    sendPortal("", F("Bitte ein WLAN auswaehlen."), 400);
    return;
  }

  Serial.print(F("[portal] Saving credentials for SSID: "));
  Serial.println(ssid);
  if (!saveCredentials(ssid, password)) {
    sendPortal("", F("Credentials konnten nicht gespeichert werden."), 500);
    return;
  }

  sendPortal(F("Credentials gespeichert. Starte den Controller neu, um das konfigurierte WLAN zu verwenden."));
}

void handleReboot() {
  Serial.println(F("[portal] Reboot requested from captive portal."));
  webServer.send(200, F("text/html"),
                 renderPage(F("<h1>Controller startet neu</h1><p class=\"message\">Der Controller wird jetzt neu gestartet.</p>")));
  delay(500);
  ESP.restart();
}

void configurePortalRoutes() {
  webServer.on(F("/"), HTTP_GET, []() { sendPortal(); });
  webServer.on(F("/save"), HTTP_POST, handleSaveCredentials);
  webServer.on(F("/reboot"), HTTP_POST, handleReboot);
  webServer.on(F("/generate_204"), HTTP_GET, []() { sendPortal(); });
  webServer.on(F("/gen_204"), HTTP_GET, []() { sendPortal(); });
  webServer.on(F("/hotspot-detect.html"), HTTP_GET, []() { sendPortal(); });
  webServer.on(F("/connecttest.txt"), HTTP_GET, []() { sendPortal(); });
  webServer.on(F("/ncsi.txt"), HTTP_GET, []() { sendPortal(); });
  webServer.onNotFound(redirectToPortal);
}

void startProvisioningMode() {
  provisioningActive = true;
  Serial.println(F("[ap] Starting provisioning mode."));
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(kApIp, kApGateway, kApSubnet);
  const bool apStarted = WiFi.softAP(kApSsid);

  dnsServer.start(53, F("*"), kApIp);
  configurePortalRoutes();
  webServer.begin();

  Serial.print(F("[ap] Provisioning AP start result: "));
  Serial.println(apStarted ? F("ok") : F("failed"));
  Serial.print(F("[ap] SSID: "));
  Serial.println(kApSsid);
  Serial.print(F("[ap] IP: "));
  Serial.println(kApIp);
  Serial.println(F("[portal] HTTP server and DNS captive portal are running."));
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("[boot] wifi-i2c controller starting."));

  if (!connectToConfiguredWifi()) {
    startProvisioningMode();
  } else {
    Serial.println(F("[boot] Normal WiFi station mode active."));
  }
}

void loop() {
  if (provisioningActive) {
    dnsServer.processNextRequest();
    webServer.handleClient();
  }
}
