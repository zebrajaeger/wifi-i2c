## Why

Der Controller soll ohne vorab bekannte Netzwerkumgebung in Betrieb genommen werden koennen. Wenn kein konfiguriertes WLAN vorhanden oder erreichbar ist, braucht der Nutzer einen einfachen lokalen Weg, um WLAN-Zugangsdaten zu hinterlegen und den Controller anschliessend in das Zielnetz zu bringen.

## What Changes

- Der Controller versucht beim Start, sich mit gespeicherten WLAN-Zugangsdaten zu verbinden.
- Wenn keine Zugangsdaten gespeichert sind oder das konfigurierte WLAN nicht erreichbar ist, startet der Controller einen eigenen Access Point.
- Verbindet sich ein Nutzer, z. B. per Smartphone, mit diesem Access Point, wird ein Captive Portal bereitgestellt.
- Das Captive Portal zeigt die gefundenen WLANs aus der Umgebung als SSID-Liste, sortiert nach Signalstaerke absteigend.
- Der Nutzer kann eine SSID auswaehlen, ein Passwort eingeben und die Zugangsdaten speichern.
- Das Portal bietet einen Neustart-Button, damit der Controller neu startet und sich automatisch mit dem gespeicherten WLAN verbindet.

## Capabilities

### New Capabilities
- `wifi-provisioning-captive-portal`: WLAN-Verbindungsaufbau mit AP-Fallback, Captive-Portal-Konfiguration, sortierter SSID-Auswahl, Credential-Speicherung und Neustart.

### Modified Capabilities

## Impact

- Betrifft die Firmware-Startlogik fuer WLAN-Verbindungsaufbau und Fallback-Verhalten.
- Erfordert Access-Point-, DNS/HTTP-Captive-Portal- und WLAN-Scan-Funktionalitaet auf dem Controller.
- Erfordert dauerhafte Speicherung der WLAN-Zugangsdaten.
- Erfordert eine Weboberflaeche fuer SSID-Auswahl, Passworteingabe, Speichern und Neustart.
