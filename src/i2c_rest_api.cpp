#include "i2c_rest_api.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Wire.h>

#include <vector>

namespace I2cRestApi {
namespace {
constexpr uint8_t kI2cMinAddress = 0x03;
constexpr uint8_t kI2cMaxAddress = 0x77;
constexpr size_t kMaxI2cPayloadBytes = 32;
constexpr size_t kJsonDocumentBytes = 1024;

WebServer *webServer = nullptr;

String hexByte(uint8_t value) {
  String hex = F("0x");
  if (value < 0x10) {
    hex += '0';
  }
  hex += String(value, HEX);
  hex.toUpperCase();
  return hex;
}

void sendJson(int statusCode, const String &body) {
  webServer->send(statusCode, F("application/json"), body);
}

void sendJsonDocument(int statusCode, const JsonDocument &document) {
  String body;
  serializeJson(document, body);
  sendJson(statusCode, body);
}

void sendJsonError(int statusCode, const __FlashStringHelper *error,
                   const String &detail = "") {
  DynamicJsonDocument response(256);
  response[F("ok")] = false;
  response[F("error")] = error;
  if (detail.length() > 0) {
    response[F("detail")] = detail;
  }

  sendJsonDocument(statusCode, response);
}

bool parseJsonRequest(DynamicJsonDocument &document) {
  const String body = webServer->arg(F("plain"));
  if (body.length() == 0) {
    sendJsonError(400, F("empty_body"));
    return false;
  }

  const DeserializationError error = deserializeJson(document, body);
  if (error) {
    sendJsonError(400, F("malformed_json"), error.c_str());
    return false;
  }

  return true;
}

bool parseIntegerLike(JsonVariant value, long &parsedValue) {
  if (value.is<int>() || value.is<long>()) {
    parsedValue = value.as<long>();
    return true;
  }

  if (!value.is<const char *>()) {
    return false;
  }

  const char *raw = value.as<const char *>();
  if (raw == nullptr || raw[0] == '\0') {
    return false;
  }

  char *end = nullptr;
  parsedValue = strtol(raw, &end, 0);
  return end != raw && end != nullptr && *end == '\0';
}

bool parseAddress(JsonVariant value, uint8_t &address) {
  long parsedAddress = 0;
  if (!parseIntegerLike(value, parsedAddress)) {
    sendJsonError(400, F("invalid_address"),
                  F("address must be a number or 0x-prefixed string"));
    return false;
  }

  if (parsedAddress < kI2cMinAddress || parsedAddress > kI2cMaxAddress) {
    sendJsonError(400, F("address_out_of_range"),
                  String(F("address must be between ")) + hexByte(kI2cMinAddress) +
                      F(" and ") + hexByte(kI2cMaxAddress));
    return false;
  }

  address = static_cast<uint8_t>(parsedAddress);
  return true;
}

bool parseLength(JsonVariant value, size_t &length) {
  long parsedLength = 0;
  if (!parseIntegerLike(value, parsedLength)) {
    sendJsonError(400, F("invalid_length"), F("length must be a number"));
    return false;
  }

  if (parsedLength < 1 ||
      parsedLength > static_cast<long>(kMaxI2cPayloadBytes)) {
    sendJsonError(400, F("length_out_of_range"),
                  String(F("length must be between 1 and ")) + kMaxI2cPayloadBytes);
    return false;
  }

  length = static_cast<size_t>(parsedLength);
  return true;
}

bool parseByteArray(JsonVariant value, const __FlashStringHelper *fieldName,
                    bool allowEmpty, std::vector<uint8_t> &bytes) {
  JsonArray array = value.as<JsonArray>();
  if (array.isNull()) {
    sendJsonError(400, F("invalid_bytes"),
                  String(fieldName) + F(" must be an array of byte values"));
    return false;
  }

  if (!allowEmpty && array.size() == 0) {
    sendJsonError(400, F("empty_bytes"), String(fieldName) + F(" must not be empty"));
    return false;
  }

  if (array.size() > kMaxI2cPayloadBytes) {
    sendJsonError(400, F("payload_too_large"),
                  String(fieldName) + F(" must contain at most ") +
                      kMaxI2cPayloadBytes + F(" bytes"));
    return false;
  }

  bytes.clear();
  bytes.reserve(array.size());
  for (JsonVariant item : array) {
    long parsedByte = 0;
    if (!parseIntegerLike(item, parsedByte) || parsedByte < 0 || parsedByte > 255) {
      sendJsonError(400, F("invalid_byte"),
                    String(fieldName) + F(" values must be between 0 and 255"));
      return false;
    }
    bytes.push_back(static_cast<uint8_t>(parsedByte));
  }

  return true;
}

String bytesToJson(const std::vector<uint8_t> &bytes) {
  String json = F("[");
  for (size_t i = 0; i < bytes.size(); ++i) {
    if (i > 0) {
      json += ',';
    }
    json += bytes[i];
  }
  json += ']';
  return json;
}

String i2cStatusLabel(uint8_t status) {
  switch (status) {
    case 0:
      return F("ok");
    case 1:
      return F("data_too_long");
    case 2:
      return F("address_nack");
    case 3:
      return F("data_nack");
    case 4:
      return F("other_error");
    case 5:
      return F("timeout");
    default:
      return F("unknown_error");
  }
}

std::vector<uint8_t> scanI2cBus() {
  std::vector<uint8_t> devices;
  Serial.println(F("[i2c] Starting bus scan."));

  for (uint8_t address = kI2cMinAddress; address <= kI2cMaxAddress; ++address) {
    Wire.beginTransmission(address);
    const uint8_t status = Wire.endTransmission();
    if (status == 0) {
      devices.push_back(address);
      Serial.print(F("[i2c] Found device at "));
      Serial.println(hexByte(address));
    }
  }

  Serial.print(F("[i2c] Scan complete. Devices: "));
  Serial.println(devices.size());
  return devices;
}

uint8_t writeI2cBytes(uint8_t address, const std::vector<uint8_t> &bytes,
                      bool sendStop = true) {
  Wire.beginTransmission(address);
  for (uint8_t value : bytes) {
    Wire.write(value);
  }
  return Wire.endTransmission(sendStop);
}

std::vector<uint8_t> readI2cBytes(uint8_t address, size_t length) {
  std::vector<uint8_t> bytes;
  bytes.reserve(length);

  const uint8_t received = Wire.requestFrom(address, static_cast<uint8_t>(length));
  while (Wire.available() > 0 && bytes.size() < length) {
    bytes.push_back(static_cast<uint8_t>(Wire.read()));
  }

  Serial.print(F("[i2c] Requested "));
  Serial.print(length);
  Serial.print(F(" bytes from "));
  Serial.print(hexByte(address));
  Serial.print(F(", received "));
  Serial.println(received);
  return bytes;
}

void handleI2cScan() {
  const std::vector<uint8_t> devices = scanI2cBus();
  String body = F("{\"ok\":true,\"devices\":[");

  for (size_t i = 0; i < devices.size(); ++i) {
    if (i > 0) {
      body += ',';
    }
    body += F("{\"address\":");
    body += devices[i];
    body += F(",\"hex\":\"");
    body += hexByte(devices[i]);
    body += F("\"}");
  }

  body += F("],\"count\":");
  body += devices.size();
  body += F("}");
  sendJson(200, body);
}

void handleI2cWrite() {
  DynamicJsonDocument request(kJsonDocumentBytes);
  if (!parseJsonRequest(request)) {
    return;
  }

  uint8_t address = 0;
  std::vector<uint8_t> bytes;
  if (!parseAddress(request[F("address")], address) ||
      !parseByteArray(request[F("bytes")], F("bytes"), false, bytes)) {
    return;
  }

  Serial.print(F("[api] I2C write to "));
  Serial.print(hexByte(address));
  Serial.print(F(" bytes="));
  Serial.println(bytes.size());

  const uint8_t status = writeI2cBytes(address, bytes);
  DynamicJsonDocument response(256);
  response[F("ok")] = status == 0;
  response[F("address")] = address;
  response[F("hex")] = hexByte(address);
  response[F("bytesWritten")] = bytes.size();
  response[F("i2cStatus")] = status;
  response[F("status")] = i2cStatusLabel(status);
  if (status != 0) {
    response[F("error")] = F("i2c_write_failed");
  }

  sendJsonDocument(status == 0 ? 200 : 502, response);
}

void handleI2cRead() {
  DynamicJsonDocument request(kJsonDocumentBytes);
  if (!parseJsonRequest(request)) {
    return;
  }

  uint8_t address = 0;
  size_t length = 0;
  if (!parseAddress(request[F("address")], address) ||
      !parseLength(request[F("length")], length)) {
    return;
  }

  Serial.print(F("[api] I2C read from "));
  Serial.print(hexByte(address));
  Serial.print(F(" length="));
  Serial.println(length);

  const std::vector<uint8_t> bytes = readI2cBytes(address, length);
  const bool complete = bytes.size() == length;
  String body = F("{\"ok\":");
  body += complete ? F("true") : F("false");
  body += F(",\"address\":");
  body += address;
  body += F(",\"hex\":\"");
  body += hexByte(address);
  body += F("\",\"requested\":");
  body += length;
  body += F(",\"received\":");
  body += bytes.size();
  body += F(",\"bytes\":");
  body += bytesToJson(bytes);
  if (!complete) {
    body += F(",\"error\":\"i2c_read_incomplete\"");
  }
  body += F("}");

  sendJson(complete ? 200 : 502, body);
}

void handleI2cWriteRead() {
  DynamicJsonDocument request(kJsonDocumentBytes);
  if (!parseJsonRequest(request)) {
    return;
  }

  uint8_t address = 0;
  size_t length = 0;
  std::vector<uint8_t> prefix;
  if (!parseAddress(request[F("address")], address) ||
      !parseByteArray(request[F("prefix")], F("prefix"), false, prefix) ||
      !parseLength(request[F("length")], length)) {
    return;
  }

  Serial.print(F("[api] I2C write-read at "));
  Serial.print(hexByte(address));
  Serial.print(F(" prefix="));
  Serial.print(prefix.size());
  Serial.print(F(" read="));
  Serial.println(length);

  const uint8_t status = writeI2cBytes(address, prefix, false);
  if (status != 0) {
    DynamicJsonDocument response(256);
    response[F("ok")] = false;
    response[F("address")] = address;
    response[F("hex")] = hexByte(address);
    response[F("i2cStatus")] = status;
    response[F("status")] = i2cStatusLabel(status);
    response[F("error")] = F("i2c_prefix_write_failed");
    sendJsonDocument(502, response);
    return;
  }

  const std::vector<uint8_t> bytes = readI2cBytes(address, length);
  const bool complete = bytes.size() == length;
  String body = F("{\"ok\":");
  body += complete ? F("true") : F("false");
  body += F(",\"address\":");
  body += address;
  body += F(",\"hex\":\"");
  body += hexByte(address);
  body += F("\",\"prefixWritten\":");
  body += prefix.size();
  body += F(",\"requested\":");
  body += length;
  body += F(",\"received\":");
  body += bytes.size();
  body += F(",\"bytes\":");
  body += bytesToJson(bytes);
  if (!complete) {
    body += F(",\"error\":\"i2c_read_incomplete\"");
  }
  body += F("}");

  sendJson(complete ? 200 : 502, body);
}

void handleApiNotFound() {
  sendJsonError(404, F("not_found"), webServer->uri());
}

void configureRoutes() {
  webServer->on(F("/api/i2c/scan"), HTTP_GET, handleI2cScan);
  webServer->on(F("/api/i2c/write"), HTTP_POST, handleI2cWrite);
  webServer->on(F("/api/i2c/read"), HTTP_POST, handleI2cRead);
  webServer->on(F("/api/i2c/write-read"), HTTP_POST, handleI2cWriteRead);
  webServer->onNotFound(handleApiNotFound);
}
}  // namespace

void begin(WebServer &server) {
  webServer = &server;
  Wire.begin();
  configureRoutes();

  Serial.println(F("[api] I2C REST API mode started."));
  Serial.print(F("[api] Base URL: http://"));
  Serial.println(WiFi.localIP());
  Serial.print(F("[i2c] Max payload bytes: "));
  Serial.println(kMaxI2cPayloadBytes);
}

}  // namespace I2cRestApi
