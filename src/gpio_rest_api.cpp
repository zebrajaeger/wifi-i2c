#include "gpio_rest_api.h"

#include <Arduino.h>
#include <ArduinoJson.h>

namespace GpioRestApi {
namespace {
constexpr size_t kJsonDocumentBytes = 1024;
constexpr uint8_t kAnalogResolutionBits = 12;
constexpr int kAnalogMaxRaw = (1 << kAnalogResolutionBits) - 1;

struct PinDefinition {
  uint8_t pin;
  bool input;
  bool output;
  bool pullup;
  bool pulldown;
  bool analog;
  const __FlashStringHelper *label;
};

struct PinState {
  bool configured;
  const __FlashStringHelper *mode;
  bool pullup;
  bool pulldown;
  bool outputMode;
  int lastOutputValue;
};

constexpr PinDefinition kPins[] = {
    {13, true, true, true, true, false, F("safe_gpio")},
    {14, true, true, true, true, false, F("safe_gpio")},
    {16, true, true, true, true, false, F("safe_gpio")},
    {17, true, true, true, true, false, F("safe_gpio")},
    {18, true, true, true, true, false, F("safe_gpio")},
    {19, true, true, true, true, false, F("safe_gpio")},
    {23, true, true, true, true, false, F("safe_gpio")},
    {25, true, true, true, true, false, F("safe_gpio")},
    {26, true, true, true, true, false, F("safe_gpio")},
    {27, true, true, true, true, false, F("safe_gpio")},
    {32, true, true, true, true, true, F("safe_gpio_adc1")},
    {33, true, true, true, true, true, F("safe_gpio_adc1")},
    {34, true, false, false, false, true, F("input_only_adc1")},
    {35, true, false, false, false, true, F("input_only_adc1")},
    {36, true, false, false, false, true, F("input_only_adc1")},
    {39, true, false, false, false, true, F("input_only_adc1")},
};

PinState pinStates[sizeof(kPins) / sizeof(kPins[0])];
WebServer *webServer = nullptr;

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

  Serial.print(F("[api] GPIO request rejected: "));
  Serial.print(error);
  if (detail.length() > 0) {
    Serial.print(F(" - "));
    Serial.print(detail);
  }
  Serial.println();

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

bool parseQueryInteger(const String &value, long &parsedValue) {
  if (value.length() == 0) {
    return false;
  }

  char *end = nullptr;
  parsedValue = strtol(value.c_str(), &end, 0);
  return end != value.c_str() && end != nullptr && *end == '\0';
}

const PinDefinition *findPin(uint8_t pin, size_t *index = nullptr) {
  for (size_t i = 0; i < sizeof(kPins) / sizeof(kPins[0]); ++i) {
    if (kPins[i].pin == pin) {
      if (index != nullptr) {
        *index = i;
      }
      return &kPins[i];
    }
  }
  return nullptr;
}

bool parsePin(JsonVariant value, const PinDefinition *&pin, size_t &index) {
  long parsedPin = 0;
  if (!parseIntegerLike(value, parsedPin) || parsedPin < 0 || parsedPin > 255) {
    sendJsonError(400, F("invalid_pin"), F("pin must be a GPIO number"));
    return false;
  }

  pin = findPin(static_cast<uint8_t>(parsedPin), &index);
  if (pin == nullptr) {
    sendJsonError(400, F("unsupported_pin"),
                  String(F("pin ")) + parsedPin + F(" is not available through REST"));
    return false;
  }

  return true;
}

bool parsePinFromQuery(const PinDefinition *&pin, size_t &index) {
  if (!webServer->hasArg(F("pin"))) {
    sendJsonError(400, F("missing_pin"), F("pin query parameter is required"));
    return false;
  }

  long parsedPin = 0;
  if (!parseQueryInteger(webServer->arg(F("pin")), parsedPin) || parsedPin < 0 ||
      parsedPin > 255) {
    sendJsonError(400, F("invalid_pin"), F("pin must be a GPIO number"));
    return false;
  }

  pin = findPin(static_cast<uint8_t>(parsedPin), &index);
  if (pin == nullptr) {
    sendJsonError(400, F("unsupported_pin"),
                  String(F("pin ")) + parsedPin + F(" is not available through REST"));
    return false;
  }

  return true;
}

bool parseValue(JsonVariant value, int &parsedValue) {
  long rawValue = 0;
  if (!parseIntegerLike(value, rawValue) || (rawValue != 0 && rawValue != 1)) {
    sendJsonError(400, F("invalid_value"), F("value must be 0 or 1"));
    return false;
  }

  parsedValue = static_cast<int>(rawValue);
  return true;
}

bool modeEquals(const char *mode, const __FlashStringHelper *expected) {
  return String(mode) == String(expected);
}

bool applyMode(const PinDefinition &pin, PinState &state, const char *mode,
               bool hasInitialValue, int initialValue) {
  if (mode == nullptr || mode[0] == '\0') {
    sendJsonError(400, F("invalid_mode"), F("mode is required"));
    return false;
  }

  if (modeEquals(mode, F("input"))) {
    if (!pin.input) {
      sendJsonError(400, F("pin_not_input_capable"));
      return false;
    }
    pinMode(pin.pin, INPUT);
    state = {true, F("input"), false, false, false, -1};
    return true;
  }

  if (modeEquals(mode, F("input_pullup"))) {
    if (!pin.input || !pin.pullup) {
      sendJsonError(400, F("pin_pullup_unsupported"));
      return false;
    }
    pinMode(pin.pin, INPUT_PULLUP);
    state = {true, F("input_pullup"), true, false, false, -1};
    return true;
  }

  if (modeEquals(mode, F("input_pulldown"))) {
    if (!pin.input || !pin.pulldown) {
      sendJsonError(400, F("pin_pulldown_unsupported"));
      return false;
    }
    pinMode(pin.pin, INPUT_PULLDOWN);
    state = {true, F("input_pulldown"), false, true, false, -1};
    return true;
  }

  if (modeEquals(mode, F("output"))) {
    if (!pin.output) {
      sendJsonError(400, F("pin_not_output_capable"));
      return false;
    }
    pinMode(pin.pin, OUTPUT);
    if (hasInitialValue) {
      digitalWrite(pin.pin, initialValue == 0 ? LOW : HIGH);
    }
    state = {true, F("output"), false, false, true, hasInitialValue ? initialValue : -1};
    return true;
  }

  if (modeEquals(mode, F("output_open_drain"))) {
    if (!pin.output) {
      sendJsonError(400, F("pin_not_output_capable"));
      return false;
    }
#if defined(OUTPUT_OPEN_DRAIN)
    pinMode(pin.pin, OUTPUT_OPEN_DRAIN);
    if (hasInitialValue) {
      digitalWrite(pin.pin, initialValue == 0 ? LOW : HIGH);
    }
    state = {true, F("output_open_drain"), false, false, true,
             hasInitialValue ? initialValue : -1};
    return true;
#else
    sendJsonError(400, F("mode_unsupported"),
                  F("output_open_drain is not supported by this platform"));
    return false;
#endif
  }

  sendJsonError(400, F("invalid_mode"),
                F("mode must be input, input_pullup, input_pulldown, output, or output_open_drain"));
  return false;
}

void addPinState(JsonObject target, const PinDefinition &pin, const PinState &state) {
  const int value = digitalRead(pin.pin);

  target[F("pin")] = pin.pin;
  target[F("label")] = pin.label;
  target[F("available")] = true;
  target[F("inputCapable")] = pin.input;
  target[F("outputCapable")] = pin.output;
  target[F("pullupCapable")] = pin.pullup;
  target[F("pulldownCapable")] = pin.pulldown;
  target[F("analogCapable")] = pin.analog;
  target[F("mode")] = state.configured ? state.mode : F("unconfigured");
  target[F("pullup")] = state.pullup;
  target[F("pulldown")] = state.pulldown;
  target[F("value")] = value == HIGH ? 1 : 0;
  if (state.lastOutputValue >= 0) {
    target[F("lastOutputValue")] = state.lastOutputValue;
  } else {
    target[F("lastOutputValue")] = nullptr;
  }
}

void addAnalogReading(JsonObject target, const PinDefinition &pin) {
  const int raw = analogRead(pin.pin);

  target[F("pin")] = pin.pin;
  target[F("raw")] = raw;
  target[F("resolutionBits")] = kAnalogResolutionBits;
  target[F("maxRaw")] = kAnalogMaxRaw;
#if defined(ARDUINO_ARCH_ESP32)
  target[F("millivolts")] = analogReadMilliVolts(pin.pin);
#endif
}

void handleGpioList() {
  DynamicJsonDocument response(4096);
  response[F("ok")] = true;
  JsonArray pins = response.createNestedArray(F("pins"));

  for (size_t i = 0; i < sizeof(kPins) / sizeof(kPins[0]); ++i) {
    addPinState(pins.createNestedObject(), kPins[i], pinStates[i]);
  }

  response[F("count")] = sizeof(kPins) / sizeof(kPins[0]);
  sendJsonDocument(200, response);
}

void handleGpioRead() {
  const PinDefinition *pin = nullptr;
  size_t index = 0;
  if (!parsePinFromQuery(pin, index)) {
    return;
  }

  if (!pin->input) {
    sendJsonError(400, F("pin_not_readable"));
    return;
  }

  PinState &state = pinStates[index];
  if (!state.configured) {
    Serial.print(F("[api] GPIO auto-configure pin "));
    Serial.print(pin->pin);
    Serial.println(F(" as input for read"));
    if (!applyMode(*pin, state, "input", false, 0)) {
      return;
    }
  }

  Serial.print(F("[api] GPIO read pin "));
  Serial.println(pin->pin);

  DynamicJsonDocument response(512);
  response[F("ok")] = true;
  JsonObject pinObject = response.createNestedObject(F("gpio"));
  addPinState(pinObject, *pin, state);
  sendJsonDocument(200, response);
}

void handleGpioAnalogRead() {
  const PinDefinition *pin = nullptr;
  size_t index = 0;
  if (!parsePinFromQuery(pin, index)) {
    return;
  }
  (void)index;

  if (!pin->analog) {
    sendJsonError(400, F("pin_not_analog_capable"));
    return;
  }

  Serial.print(F("[api] GPIO analog read pin "));
  Serial.println(pin->pin);

  DynamicJsonDocument response(512);
  response[F("ok")] = true;
  JsonObject analogObject = response.createNestedObject(F("analog"));
  addAnalogReading(analogObject, *pin);
  sendJsonDocument(200, response);
}

void handleGpioConfigure() {
  DynamicJsonDocument request(kJsonDocumentBytes);
  if (!parseJsonRequest(request)) {
    return;
  }

  const PinDefinition *pin = nullptr;
  size_t index = 0;
  if (!parsePin(request[F("pin")], pin, index)) {
    return;
  }

  const char *mode = request[F("mode")] | "";
  int initialValue = 0;
  const bool hasInitialValue = !request[F("value")].isNull();
  if (hasInitialValue && !parseValue(request[F("value")], initialValue)) {
    return;
  }

  Serial.print(F("[api] GPIO configure pin "));
  Serial.print(pin->pin);
  Serial.print(F(" mode="));
  Serial.println(mode);

  if (!applyMode(*pin, pinStates[index], mode, hasInitialValue, initialValue)) {
    return;
  }

  DynamicJsonDocument response(512);
  response[F("ok")] = true;
  JsonObject pinObject = response.createNestedObject(F("gpio"));
  addPinState(pinObject, *pin, pinStates[index]);
  sendJsonDocument(200, response);
}

void handleGpioWrite() {
  DynamicJsonDocument request(kJsonDocumentBytes);
  if (!parseJsonRequest(request)) {
    return;
  }

  const PinDefinition *pin = nullptr;
  size_t index = 0;
  int value = 0;
  if (!parsePin(request[F("pin")], pin, index) ||
      !parseValue(request[F("value")], value)) {
    return;
  }

  PinState &state = pinStates[index];
  if (!pin->output) {
    sendJsonError(400, F("pin_not_output_capable"));
    return;
  }

  if (!state.configured) {
    Serial.print(F("[api] GPIO auto-configure pin "));
    Serial.print(pin->pin);
    Serial.println(F(" as output for write"));
    if (!applyMode(*pin, state, "output", true, value)) {
      return;
    }
  } else if (!state.outputMode) {
    sendJsonError(400, F("pin_not_output_configured"),
                  F("configure the pin as output before writing"));
    return;
  }

  Serial.print(F("[api] GPIO write pin "));
  Serial.print(pin->pin);
  Serial.print(F(" value="));
  Serial.println(value);

  digitalWrite(pin->pin, value == 0 ? LOW : HIGH);
  state.lastOutputValue = value;

  DynamicJsonDocument response(512);
  response[F("ok")] = true;
  JsonObject pinObject = response.createNestedObject(F("gpio"));
  addPinState(pinObject, *pin, state);
  sendJsonDocument(200, response);
}

void configureRoutes() {
  webServer->on(F("/api/gpio"), HTTP_GET, handleGpioList);
  webServer->on(F("/api/gpio/read"), HTTP_GET, handleGpioRead);
  webServer->on(F("/api/gpio/analog"), HTTP_GET, handleGpioAnalogRead);
  webServer->on(F("/api/gpio/configure"), HTTP_POST, handleGpioConfigure);
  webServer->on(F("/api/gpio/write"), HTTP_POST, handleGpioWrite);
}

void resetRuntimeState() {
  for (size_t i = 0; i < sizeof(kPins) / sizeof(kPins[0]); ++i) {
    pinStates[i] = {false, F("unconfigured"), false, false, false, -1};
  }
}
}  // namespace

void begin(WebServer &server) {
  webServer = &server;
#if defined(ARDUINO_ARCH_ESP32)
  analogReadResolution(kAnalogResolutionBits);
#endif
  resetRuntimeState();
  configureRoutes();

  Serial.println(F("[api] GPIO REST API routes registered."));
  Serial.print(F("[gpio] Supported REST pins: "));
  Serial.println(sizeof(kPins) / sizeof(kPins[0]));
}

}  // namespace GpioRestApi
