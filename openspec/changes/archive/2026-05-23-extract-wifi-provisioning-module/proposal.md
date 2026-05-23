## Why

Die Firmware hat inzwischen zwei klar getrennte Verantwortlichkeiten: WLAN-Provisioning und I2C-REST-API. Die I2C-REST-API ist bereits modularisiert; der WiFi-AP-, Captive-Portal- und Provisioning-Code liegt aber noch in `main.cpp` und macht den Einstiegspunkt unnoetig breit.

## What Changes

- WiFi-Credential-Speicherung, Boot-Verbindungsversuch, AP-Fallback und Captive-Portal-Routen werden aus `main.cpp` in ein eigenes WiFi-Provisioning-Modul verschoben.
- Das neue Modul erhaelt eine kleine oeffentliche Schnittstelle fuer Initialisierung und Loop-Service, z. B. `begin()` und `handleClient()`.
- `main.cpp` bleibt fuer Boot-Orchestrierung und die Entscheidung zwischen Provisioning-Modus und I2C-API-Modus verantwortlich.
- Das bestehende Verhalten fuer WLAN-Verbindung, AP-Fallback, Captive Portal, SSID-Liste, Speichern und Reboot bleibt unveraendert.
- Die I2C-REST-API bleibt in ihrem eigenen Modul und wird nicht mit Provisioning-Details vermischt.

## Capabilities

### New Capabilities
- `wifi-provisioning-module-boundary`: Strukturgrenze fuer WiFi-Provisioning, die bestehendes Verhalten beibehaelt und die Implementierung aus `main.cpp` herausloest.

### Modified Capabilities

## Impact

- Betrifft `src/main.cpp` und neue Dateien wie `src/wifi_provisioning.h` und `src/wifi_provisioning.cpp`.
- Betrifft die Webserver-Lifecycle-Struktur, weil Captive-Portal-Routen weiterhin im Provisioning-Modus registriert werden muessen.
- Betrifft keine externen REST- oder Captive-Portal-Vertraege; die Aenderung ist ein interner Refactor.
- Erfordert Build- und Runtime-Pruefung, damit AP-Fallback, Captive Portal und API-Modus nach der Trennung weiter funktionieren.
