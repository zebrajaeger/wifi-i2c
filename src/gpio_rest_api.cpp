#include "gpio_rest_api.h"

#include <Arduino.h>
#include <ArduinoJson.h>

namespace GpioRestApi {
namespace {
constexpr size_t kJsonDocumentBytes = 1024;
constexpr uint8_t kAdcResolutionBits = 12;
constexpr int kAdcMaxRaw = (1 << kAdcResolutionBits) - 1;
constexpr uint8_t kDacResolutionBits = 8;
constexpr int kDacMaxRaw = (1 << kDacResolutionBits) - 1;

struct PinDefinition {
  uint8_t pin;
  bool input;
  bool output;
  bool pullup;
  bool pulldown;
  bool adc;
  bool dac;
  const __FlashStringHelper *label;
};

struct PinState {
  bool configured;
  const __FlashStringHelper *mode;
  bool pullup;
  bool pulldown;
  bool outputMode;
  int lastOutputValue;
  int lastDacValue;
};

constexpr PinDefinition kPins[] = {
    {13, true, true, true, true, false, false, F("safe_gpio")},
    {14, true, true, true, true, false, false, F("safe_gpio")},
    {16, true, true, true, true, false, false, F("safe_gpio")},
    {17, true, true, true, true, false, false, F("safe_gpio")},
    {18, true, true, true, true, false, false, F("safe_gpio")},
    {19, true, true, true, true, false, false, F("safe_gpio")},
    {23, true, true, true, true, false, false, F("safe_gpio")},
    {25, true, true, true, true, false, true, F("safe_gpio_dac")},
    {26, true, true, true, true, false, true, F("safe_gpio_dac")},
    {27, true, true, true, true, false, false, F("safe_gpio")},
    {32, true, true, true, true, true, false, F("safe_gpio_adc1")},
    {33, true, true, true, true, true, false, F("safe_gpio_adc1")},
    {34, true, false, false, false, true, false, F("input_only_adc1")},
    {35, true, false, false, false, true, false, F("input_only_adc1")},
    {36, true, false, false, false, true, false, F("input_only_adc1")},
    {39, true, false, false, false, true, false, F("input_only_adc1")},
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

bool parseDacValue(JsonVariant value, int &parsedValue) {
  long rawValue = 0;
  if (!parseIntegerLike(value, rawValue) || rawValue < 0 || rawValue > kDacMaxRaw) {
    sendJsonError(400, F("invalid_dac_value"), F("value must be an integer from 0 to 255"));
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
    state = {true, F("input"), false, false, false, -1, state.lastDacValue};
    return true;
  }

  if (modeEquals(mode, F("input_pullup"))) {
    if (!pin.input || !pin.pullup) {
      sendJsonError(400, F("pin_pullup_unsupported"));
      return false;
    }
    pinMode(pin.pin, INPUT_PULLUP);
    state = {true, F("input_pullup"), true, false, false, -1, state.lastDacValue};
    return true;
  }

  if (modeEquals(mode, F("input_pulldown"))) {
    if (!pin.input || !pin.pulldown) {
      sendJsonError(400, F("pin_pulldown_unsupported"));
      return false;
    }
    pinMode(pin.pin, INPUT_PULLDOWN);
    state = {true, F("input_pulldown"), false, true, false, -1, state.lastDacValue};
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
    state = {true, F("output"), false, false, true, hasInitialValue ? initialValue : -1,
             state.lastDacValue};
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
             hasInitialValue ? initialValue : -1, state.lastDacValue};
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
  target[F("adcCapable")] = pin.adc;
  target[F("dacCapable")] = pin.dac;
  target[F("mode")] = state.configured ? state.mode : F("unconfigured");
  target[F("pullup")] = state.pullup;
  target[F("pulldown")] = state.pulldown;
  target[F("value")] = value == HIGH ? 1 : 0;
  if (state.lastOutputValue >= 0) {
    target[F("lastOutputValue")] = state.lastOutputValue;
  } else {
    target[F("lastOutputValue")] = nullptr;
  }
  if (state.lastDacValue >= 0) {
    target[F("lastDacValue")] = state.lastDacValue;
  } else {
    target[F("lastDacValue")] = nullptr;
  }
}

void addAdcReading(JsonObject target, const PinDefinition &pin) {
  const int raw = analogRead(pin.pin);

  target[F("pin")] = pin.pin;
  target[F("raw")] = raw;
  target[F("resolutionBits")] = kAdcResolutionBits;
  target[F("maxRaw")] = kAdcMaxRaw;
#if defined(ARDUINO_ARCH_ESP32)
  target[F("millivolts")] = analogReadMilliVolts(pin.pin);
#endif
}

void handleGpioList() {
  DynamicJsonDocument response(8192);
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

void handleGpioAdcRead() {
  const PinDefinition *pin = nullptr;
  size_t index = 0;
  if (!parsePinFromQuery(pin, index)) {
    return;
  }
  (void)index;

  if (!pin->adc) {
    sendJsonError(400, F("pin_not_adc_capable"));
    return;
  }

  Serial.print(F("[api] GPIO ADC read pin "));
  Serial.println(pin->pin);

  DynamicJsonDocument response(512);
  response[F("ok")] = true;
  JsonObject adcObject = response.createNestedObject(F("adc"));
  addAdcReading(adcObject, *pin);
  sendJsonDocument(200, response);
}

void addDacResult(JsonObject target, const PinDefinition &pin, const PinState &state,
                  int value) {
  target[F("pin")] = pin.pin;
  target[F("value")] = value;
  target[F("resolutionBits")] = kDacResolutionBits;
  target[F("maxRaw")] = kDacMaxRaw;

  JsonObject gpioObject = target.createNestedObject(F("gpio"));
  addPinState(gpioObject, pin, state);
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

void handleGpioDacWrite() {
  DynamicJsonDocument request(kJsonDocumentBytes);
  if (!parseJsonRequest(request)) {
    return;
  }

  const PinDefinition *pin = nullptr;
  size_t index = 0;
  int value = 0;
  if (!parsePin(request[F("pin")], pin, index) ||
      !parseDacValue(request[F("value")], value)) {
    return;
  }

  if (!pin->dac) {
    sendJsonError(400, F("pin_not_dac_capable"));
    return;
  }

  Serial.print(F("[api] GPIO DAC write pin "));
  Serial.print(pin->pin);
  Serial.print(F(" value="));
  Serial.println(value);

#if defined(ARDUINO_ARCH_ESP32)
  dacWrite(pin->pin, static_cast<uint8_t>(value));
#else
  sendJsonError(400, F("dac_unsupported"), F("internal DAC is not supported by this platform"));
  return;
#endif

  PinState &state = pinStates[index];
  state.lastDacValue = value;

  DynamicJsonDocument response(768);
  response[F("ok")] = true;
  JsonObject dacObject = response.createNestedObject(F("dac"));
  addDacResult(dacObject, *pin, state, value);
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
  webServer->on(F("/api/gpio/adc"), HTTP_GET, handleGpioAdcRead);
  webServer->on(F("/api/gpio/configure"), HTTP_POST, handleGpioConfigure);
  webServer->on(F("/api/gpio/write"), HTTP_POST, handleGpioWrite);
  webServer->on(F("/api/gpio/dac"), HTTP_POST, handleGpioDacWrite);
}

void resetRuntimeState() {
  for (size_t i = 0; i < sizeof(kPins) / sizeof(kPins[0]); ++i) {
    pinStates[i] = {false, F("unconfigured"), false, false, false, -1, -1};
  }
}
}  // namespace

void begin(WebServer &server) {
  webServer = &server;
#if defined(ARDUINO_ARCH_ESP32)
  analogReadResolution(kAdcResolutionBits);
#endif
  resetRuntimeState();
  configureRoutes();

  Serial.println(F("[api] GPIO REST API routes registered."));
  Serial.print(F("[gpio] Supported REST pins: "));
  Serial.println(sizeof(kPins) / sizeof(kPins[0]));
}

}  // namespace GpioRestApi
