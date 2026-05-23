## Why

Der Controller soll nach der WLAN-Anbindung nicht nur konfigurierbar sein, sondern seine eigentliche Aufgabe erfuellen: externe Clients sollen ueber das Netzwerk auf den lokalen I2C-Bus zugreifen koennen. Eine REST-Schnittstelle ist dafuer ein einfacher, toolfreundlicher Einstieg, weil sie von Tests, Skripten und Browser-nahen Clients ohne Spezialprotokoll genutzt werden kann.

## What Changes

- Der Controller stellt HTTP/REST-Endpunkte fuer typische I2C-Master-Operationen bereit.
- Externe Clients koennen einen Scan des I2C-Busses ausloesen und erhalten die gefundenen Geraeteadressen.
- Externe Clients koennen Bytes an eine I2C-Adresse schreiben.
- Externe Clients koennen Bytes von einer I2C-Adresse lesen.
- Externe Clients koennen optional einen Register-/Prefix-Write vor einem Read ausfuehren, um typische Register-basierte I2C-Geraete abzufragen.
- Die REST-API liefert strukturierte JSON-Antworten mit Erfolg, Fehlern und I2C-Statusinformationen.
- Die API ist nur verfuegbar, wenn der Controller im normalen WLAN-Station-Modus laeuft; der Provisioning-AP bleibt auf WLAN-Konfiguration beschraenkt.

## Capabilities

### New Capabilities
- `i2c-rest-api`: REST-basierter Zugriff auf den lokalen I2C-Bus fuer Scan, Write, Read und kombinierte Register-Read-Ablaufe.

### Modified Capabilities

## Impact

- Betrifft die Firmware-Webserver-Struktur, weil neben dem Captive Portal API-Routen im normalen WLAN-Modus bereitgestellt werden muessen.
- Erfordert ESP32-Arduino-I2C-Funktionalitaet ueber `Wire`.
- Erfordert JSON-Request/Response-Verarbeitung, entweder leichtgewichtig manuell oder mit einer kleinen Arduino-kompatiblen JSON-Bibliothek.
- Erfordert Validierung von I2C-Adressen, Laengen und Bytewerten, um Busfehler und unkontrollierte Speicher-/Timeout-Situationen zu vermeiden.
