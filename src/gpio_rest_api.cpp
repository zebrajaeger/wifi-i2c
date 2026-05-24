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
constexpr uint8_t kMaxPwmChannels = 16;
constexpr uint8_t kMinPwmResolutionBits = 1;
constexpr uint8_t kMaxPwmResolutionBits = 16;
constexpr uint32_t kMinPwmFrequencyHz = 1;
constexpr uint32_t kMaxPwmFrequencyHz = 40000000;

struct PinDefinition {
  uint8_t pin;
  bool input;
  bool output;
  bool pullup;
  bool pulldown;
  bool adc;
  bool dac;
  bool pwm;
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
  bool pwmActive;
  int pwmChannel;
  uint32_t pwmFrequencyHz;
  uint8_t pwmResolutionBits;
  uint32_t pwmDuty;
};

constexpr PinDefinition kPins[] = {
    {13, true, true, true, true, false, false, true, F("safe_gpio")},
    {14, true, true, true, true, false, false, true, F("safe_gpio")},
    {16, true, true, true, true, false, false, true, F("safe_gpio")},
    {17, true, true, true, true, false, false, true, F("safe_gpio")},
    {18, true, true, true, true, false, false, true, F("safe_gpio")},
    {19, true, true, true, true, false, false, true, F("safe_gpio")},
    {23, true, true, true, true, false, false, true, F("safe_gpio")},
    {25, true, true, true, true, false, true, true, F("safe_gpio_dac")},
    {26, true, true, true, true, false, true, true, F("safe_gpio_dac")},
    {27, true, true, true, true, false, false, true, F("safe_gpio")},
    {32, true, true, true, true, true, false, true, F("safe_gpio_adc1")},
    {33, true, true, true, true, true, false, true, F("safe_gpio_adc1")},
    {34, true, false, false, false, true, false, false, F("input_only_adc1")},
    {35, true, false, false, false, true, false, false, F("input_only_adc1")},
    {36, true, false, false, false, true, false, false, F("input_only_adc1")},
    {39, true, false, false, false, true, false, false, F("input_only_adc1")},
};

PinState pinStates[sizeof(kPins) / sizeof(kPins[0])];
bool pwmChannelInUse[kMaxPwmChannels];
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

bool resolvePin(long parsedPin, const PinDefinition *&pin, size_t &index) {
  if (parsedPin < 0 || parsedPin > 255) {
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

bool parsePin(JsonVariant value, const PinDefinition *&pin, size_t &index) {
  long parsedPin = 0;
  if (!parseIntegerLike(value, parsedPin)) {
    sendJsonError(400, F("invalid_pin"), F("pin must be a GPIO number"));
    return false;
  }

  return resolvePin(parsedPin, pin, index);
}

bool parsePinFromQuery(const PinDefinition *&pin, size_t &index) {
  if (!webServer->hasArg(F("pin"))) {
    sendJsonError(400, F("missing_pin"), F("pin query parameter is required"));
    return false;
  }

  long parsedPin = 0;
  if (!parseQueryInteger(webServer->arg(F("pin")), parsedPin)) {
    sendJsonError(400, F("invalid_pin"), F("pin must be a GPIO number"));
    return false;
  }

  return resolvePin(parsedPin, pin, index);
}

bool parsePinFromPath(const String &value, const PinDefinition *&pin, size_t &index) {
  long parsedPin = 0;
  if (!parseQueryInteger(value, parsedPin)) {
    sendJsonError(400, F("invalid_pin"), F("pin must be a GPIO number"));
    return false;
  }

  return resolvePin(parsedPin, pin, index);
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

uint32_t pwmMaxDuty(uint8_t resolutionBits) {
  return (1UL << resolutionBits) - 1UL;
}

bool parsePwmFrequency(JsonVariant value, uint32_t &frequencyHz) {
  long rawValue = 0;
  if (!parseIntegerLike(value, rawValue) || rawValue < kMinPwmFrequencyHz ||
      static_cast<uint32_t>(rawValue) > kMaxPwmFrequencyHz) {
    sendJsonError(400, F("invalid_pwm_frequency"),
                  F("frequencyHz must be an integer from 1 to 40000000"));
    return false;
  }

  frequencyHz = static_cast<uint32_t>(rawValue);
  return true;
}

bool parsePwmResolution(JsonVariant value, uint8_t &resolutionBits) {
  long rawValue = 0;
  if (!parseIntegerLike(value, rawValue) || rawValue < kMinPwmResolutionBits ||
      rawValue > kMaxPwmResolutionBits) {
    sendJsonError(400, F("invalid_pwm_resolution"),
                  F("resolutionBits must be an integer from 1 to 16"));
    return false;
  }

  resolutionBits = static_cast<uint8_t>(rawValue);
  return true;
}

bool parsePwmDuty(JsonVariant value, uint8_t resolutionBits, uint32_t &duty) {
  long rawValue = 0;
  const uint32_t maxDuty = pwmMaxDuty(resolutionBits);
  if (!parseIntegerLike(value, rawValue) || rawValue < 0 ||
      static_cast<uint32_t>(rawValue) > maxDuty) {
    sendJsonError(400, F("invalid_pwm_duty"),
                  String(F("duty must be an integer from 0 to ")) + maxDuty);
    return false;
  }

  duty = static_cast<uint32_t>(rawValue);
  return true;
}

bool modeEquals(const char *mode, const __FlashStringHelper *expected) {
  return String(mode) == String(expected);
}

void clearPwmState(PinState &state) {
  state.pwmActive = false;
  state.pwmChannel = -1;
  state.pwmFrequencyHz = 0;
  state.pwmResolutionBits = 0;
  state.pwmDuty = 0;
}

void detachPwm(PinState &state, uint8_t pin) {
  if (!state.pwmActive) {
    clearPwmState(state);
    return;
  }

#if defined(ARDUINO_ARCH_ESP32)
  ledcDetachPin(pin);
  if (state.pwmChannel >= 0 && state.pwmChannel < kMaxPwmChannels) {
    ledcWrite(static_cast<uint8_t>(state.pwmChannel), 0);
    pwmChannelInUse[state.pwmChannel] = false;
  }
#endif
  clearPwmState(state);
}

int findAvailablePwmChannel() {
  for (uint8_t channel = 0; channel < kMaxPwmChannels; ++channel) {
    if (!pwmChannelInUse[channel]) {
      return channel;
    }
  }
  return -1;
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
    detachPwm(state, pin.pin);
    pinMode(pin.pin, INPUT);
    state = {true, F("input"), false, false, false, -1, state.lastDacValue,
             false, -1, 0, 0, 0};
    return true;
  }

  if (modeEquals(mode, F("input_pullup"))) {
    if (!pin.input || !pin.pullup) {
      sendJsonError(400, F("pin_pullup_unsupported"));
      return false;
    }
    detachPwm(state, pin.pin);
    pinMode(pin.pin, INPUT_PULLUP);
    state = {true, F("input_pullup"), true, false, false, -1, state.lastDacValue,
             false, -1, 0, 0, 0};
    return true;
  }

  if (modeEquals(mode, F("input_pulldown"))) {
    if (!pin.input || !pin.pulldown) {
      sendJsonError(400, F("pin_pulldown_unsupported"));
      return false;
    }
    detachPwm(state, pin.pin);
    pinMode(pin.pin, INPUT_PULLDOWN);
    state = {true, F("input_pulldown"), false, true, false, -1, state.lastDacValue,
             false, -1, 0, 0, 0};
    return true;
  }

  if (modeEquals(mode, F("output"))) {
    if (!pin.output) {
      sendJsonError(400, F("pin_not_output_capable"));
      return false;
    }
    detachPwm(state, pin.pin);
    pinMode(pin.pin, OUTPUT);
    if (hasInitialValue) {
      digitalWrite(pin.pin, initialValue == 0 ? LOW : HIGH);
    }
    state = {true, F("output"), false, false, true, hasInitialValue ? initialValue : -1,
             state.lastDacValue, false, -1, 0, 0, 0};
    return true;
  }

  if (modeEquals(mode, F("output_open_drain"))) {
    if (!pin.output) {
      sendJsonError(400, F("pin_not_output_capable"));
      return false;
    }
#if defined(OUTPUT_OPEN_DRAIN)
    detachPwm(state, pin.pin);
    pinMode(pin.pin, OUTPUT_OPEN_DRAIN);
    if (hasInitialValue) {
      digitalWrite(pin.pin, initialValue == 0 ? LOW : HIGH);
    }
    state = {true, F("output_open_drain"), false, false, true,
             hasInitialValue ? initialValue : -1, state.lastDacValue, false, -1, 0, 0, 0};
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
  target[F("pwmCapable")] = pin.pwm;
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

  JsonObject pwmObject = target.createNestedObject(F("pwm"));
  pwmObject[F("active")] = state.pwmActive;
  if (state.pwmActive) {
    pwmObject[F("frequencyHz")] = state.pwmFrequencyHz;
    pwmObject[F("resolutionBits")] = state.pwmResolutionBits;
    pwmObject[F("duty")] = state.pwmDuty;
    pwmObject[F("maxDuty")] = pwmMaxDuty(state.pwmResolutionBits);
    pwmObject[F("channel")] = state.pwmChannel;
  } else {
    pwmObject[F("frequencyHz")] = nullptr;
    pwmObject[F("resolutionBits")] = nullptr;
    pwmObject[F("duty")] = nullptr;
    pwmObject[F("maxDuty")] = nullptr;
    pwmObject[F("channel")] = nullptr;
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
  DynamicJsonDocument response(12288);
  response[F("ok")] = true;
  JsonArray pins = response.createNestedArray(F("pins"));

  for (size_t i = 0; i < sizeof(kPins) / sizeof(kPins[0]); ++i) {
    addPinState(pins.createNestedObject(), kPins[i], pinStates[i]);
  }

  response[F("count")] = sizeof(kPins) / sizeof(kPins[0]);
  sendJsonDocument(200, response);
}

void performGpioRead(const PinDefinition &pin, size_t index) {
  if (!pin.input) {
    sendJsonError(400, F("pin_not_readable"));
    return;
  }

  PinState &state = pinStates[index];
  if (!state.configured) {
    Serial.print(F("[api] GPIO auto-configure pin "));
    Serial.print(pin.pin);
    Serial.println(F(" as input for read"));
    if (!applyMode(pin, state, "input", false, 0)) {
      return;
    }
  }

  Serial.print(F("[api] GPIO read pin "));
  Serial.println(pin.pin);

  DynamicJsonDocument response(1536);
  response[F("ok")] = true;
  JsonObject pinObject = response.createNestedObject(F("gpio"));
  addPinState(pinObject, pin, state);
  sendJsonDocument(200, response);
}

void handleGpioRead() {
  const PinDefinition *pin = nullptr;
  size_t index = 0;
  if (!parsePinFromQuery(pin, index)) {
    return;
  }

  performGpioRead(*pin, index);
}

void performGpioAdcRead(const PinDefinition &pin) {
  if (!pin.adc) {
    sendJsonError(400, F("pin_not_adc_capable"));
    return;
  }

  Serial.print(F("[api] GPIO ADC read pin "));
  Serial.println(pin.pin);

  DynamicJsonDocument response(1536);
  response[F("ok")] = true;
  JsonObject adcObject = response.createNestedObject(F("adc"));
  addAdcReading(adcObject, pin);
  sendJsonDocument(200, response);
}

void handleGpioAdcRead() {
  const PinDefinition *pin = nullptr;
  size_t index = 0;
  if (!parsePinFromQuery(pin, index)) {
    return;
  }
  (void)index;

  performGpioAdcRead(*pin);
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

void addPwmState(JsonObject target, const PinDefinition &pin, const PinState &state) {
  target[F("pin")] = pin.pin;
  target[F("active")] = state.pwmActive;
  if (state.pwmActive) {
    target[F("frequencyHz")] = state.pwmFrequencyHz;
    target[F("resolutionBits")] = state.pwmResolutionBits;
    target[F("duty")] = state.pwmDuty;
    target[F("maxDuty")] = pwmMaxDuty(state.pwmResolutionBits);
    target[F("channel")] = state.pwmChannel;
  } else {
    target[F("frequencyHz")] = nullptr;
    target[F("resolutionBits")] = nullptr;
    target[F("duty")] = nullptr;
    target[F("maxDuty")] = nullptr;
    target[F("channel")] = nullptr;
  }

  JsonObject gpioObject = target.createNestedObject(F("gpio"));
  addPinState(gpioObject, pin, state);
}

void performGpioConfigure(const PinDefinition &pin, size_t index,
                          DynamicJsonDocument &request) {
  const char *mode = request[F("mode")] | "";
  int initialValue = 0;
  const bool hasInitialValue = !request[F("value")].isNull();
  if (hasInitialValue && !parseValue(request[F("value")], initialValue)) {
    return;
  }

  Serial.print(F("[api] GPIO configure pin "));
  Serial.print(pin.pin);
  Serial.print(F(" mode="));
  Serial.println(mode);

  if (!applyMode(pin, pinStates[index], mode, hasInitialValue, initialValue)) {
    return;
  }

  DynamicJsonDocument response(512);
  response[F("ok")] = true;
  JsonObject pinObject = response.createNestedObject(F("gpio"));
  addPinState(pinObject, pin, pinStates[index]);
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

  performGpioConfigure(*pin, index, request);
}

void performGpioDacWrite(const PinDefinition &pin, size_t index,
                         DynamicJsonDocument &request) {
  int value = 0;
  if (!parseDacValue(request[F("value")], value)) {
    return;
  }

  if (!pin.dac) {
    sendJsonError(400, F("pin_not_dac_capable"));
    return;
  }

  Serial.print(F("[api] GPIO DAC write pin "));
  Serial.print(pin.pin);
  Serial.print(F(" value="));
  Serial.println(value);

  PinState &state = pinStates[index];
  if (state.pwmActive) {
    detachPwm(state, pin.pin);
    state.configured = false;
    state.mode = F("unconfigured");
    state.pullup = false;
    state.pulldown = false;
    state.outputMode = false;
    state.lastOutputValue = -1;
  }

#if defined(ARDUINO_ARCH_ESP32)
  dacWrite(pin.pin, static_cast<uint8_t>(value));
#else
  sendJsonError(400, F("dac_unsupported"), F("internal DAC is not supported by this platform"));
  return;
#endif

  state.lastDacValue = value;

  DynamicJsonDocument response(2048);
  response[F("ok")] = true;
  JsonObject dacObject = response.createNestedObject(F("dac"));
  addDacResult(dacObject, pin, state, value);
  sendJsonDocument(200, response);
}

void handleGpioDacWrite() {
  DynamicJsonDocument request(kJsonDocumentBytes);
  if (!parseJsonRequest(request)) {
    return;
  }

  const PinDefinition *pin = nullptr;
  size_t index = 0;
  if (!parsePin(request[F("pin")], pin, index)) {
    return;
  }

  performGpioDacWrite(*pin, index, request);
}

void performGpioPwmRead(const PinDefinition &pin, size_t index) {
  PinState &state = pinStates[index];
  if (!pin.pwm) {
    sendJsonError(400, F("pin_not_pwm_capable"));
    return;
  }

  DynamicJsonDocument response(2048);
  response[F("ok")] = true;
  JsonObject pwmObject = response.createNestedObject(F("pwm"));
  addPwmState(pwmObject, pin, state);
  sendJsonDocument(200, response);
}

void handleGpioPwmRead() {
  const PinDefinition *pin = nullptr;
  size_t index = 0;
  if (!parsePinFromQuery(pin, index)) {
    return;
  }

  performGpioPwmRead(*pin, index);
}

void performGpioPwmWrite(const PinDefinition &pin, size_t index,
                         DynamicJsonDocument &request) {
  if (!pin.pwm) {
    sendJsonError(400, F("pin_not_pwm_capable"));
    return;
  }

  uint32_t frequencyHz = 0;
  uint8_t resolutionBits = 0;
  uint32_t duty = 0;
  if (!parsePwmFrequency(request[F("frequencyHz")], frequencyHz) ||
      !parsePwmResolution(request[F("resolutionBits")], resolutionBits) ||
      !parsePwmDuty(request[F("duty")], resolutionBits, duty)) {
    return;
  }

  PinState &state = pinStates[index];
  int channel = state.pwmActive ? state.pwmChannel : findAvailablePwmChannel();
  if (channel < 0) {
    sendJsonError(409, F("pwm_channel_unavailable"));
    return;
  }

  Serial.print(F("[api] GPIO PWM pin "));
  Serial.print(pin.pin);
  Serial.print(F(" freq="));
  Serial.print(frequencyHz);
  Serial.print(F(" resolution="));
  Serial.print(resolutionBits);
  Serial.print(F(" duty="));
  Serial.println(duty);

#if defined(ARDUINO_ARCH_ESP32)
  const double actualFrequency = ledcSetup(static_cast<uint8_t>(channel), frequencyHz,
                                           resolutionBits);
  if (actualFrequency <= 0) {
    sendJsonError(400, F("pwm_setup_failed"));
    return;
  }
  ledcAttachPin(pin.pin, static_cast<uint8_t>(channel));
  ledcWrite(static_cast<uint8_t>(channel), duty);
#else
  sendJsonError(400, F("pwm_unsupported"), F("hardware PWM is not supported by this platform"));
  return;
#endif

  pwmChannelInUse[channel] = true;
  state.configured = true;
  state.mode = F("pwm");
  state.pullup = false;
  state.pulldown = false;
  state.outputMode = true;
  state.lastOutputValue = -1;
  state.pwmActive = true;
  state.pwmChannel = channel;
  state.pwmFrequencyHz = frequencyHz;
  state.pwmResolutionBits = resolutionBits;
  state.pwmDuty = duty;

  DynamicJsonDocument response(2048);
  response[F("ok")] = true;
  JsonObject pwmObject = response.createNestedObject(F("pwm"));
  addPwmState(pwmObject, pin, state);
  sendJsonDocument(200, response);
}

void handleGpioPwmWrite() {
  DynamicJsonDocument request(kJsonDocumentBytes);
  if (!parseJsonRequest(request)) {
    return;
  }

  const PinDefinition *pin = nullptr;
  size_t index = 0;
  if (!parsePin(request[F("pin")], pin, index)) {
    return;
  }

  performGpioPwmWrite(*pin, index, request);
}

void performGpioPwmStop(const PinDefinition &pin, size_t index) {
  if (!pin.pwm) {
    sendJsonError(400, F("pin_not_pwm_capable"));
    return;
  }

  PinState &state = pinStates[index];
  Serial.print(F("[api] GPIO PWM stop pin "));
  Serial.println(pin.pin);
  detachPwm(state, pin.pin);
  state.configured = false;
  state.mode = F("unconfigured");
  state.pullup = false;
  state.pulldown = false;
  state.outputMode = false;
  state.lastOutputValue = -1;

  DynamicJsonDocument response(1536);
  response[F("ok")] = true;
  JsonObject pinObject = response.createNestedObject(F("gpio"));
  addPinState(pinObject, pin, state);
  sendJsonDocument(200, response);
}

void handleGpioPwmStop() {
  DynamicJsonDocument request(kJsonDocumentBytes);
  if (!parseJsonRequest(request)) {
    return;
  }

  const PinDefinition *pin = nullptr;
  size_t index = 0;
  if (!parsePin(request[F("pin")], pin, index)) {
    return;
  }

  performGpioPwmStop(*pin, index);
}

void performGpioWrite(const PinDefinition &pin, size_t index, DynamicJsonDocument &request) {
  int value = 0;
  if (!parseValue(request[F("value")], value)) {
    return;
  }

  PinState &state = pinStates[index];
  if (!pin.output) {
    sendJsonError(400, F("pin_not_output_capable"));
    return;
  }

  if (!state.configured) {
    Serial.print(F("[api] GPIO auto-configure pin "));
    Serial.print(pin.pin);
    Serial.println(F(" as output for write"));
    if (!applyMode(pin, state, "output", true, value)) {
      return;
    }
  } else if (!state.outputMode) {
    sendJsonError(400, F("pin_not_output_configured"),
                  F("configure the pin as output before writing"));
    return;
  }

  if (state.pwmActive) {
    detachPwm(state, pin.pin);
    state.configured = true;
    state.mode = F("output");
    state.pullup = false;
    state.pulldown = false;
    state.outputMode = true;
  }

  Serial.print(F("[api] GPIO write pin "));
  Serial.print(pin.pin);
  Serial.print(F(" value="));
  Serial.println(value);

  digitalWrite(pin.pin, value == 0 ? LOW : HIGH);
  state.lastOutputValue = value;

  DynamicJsonDocument response(1536);
  response[F("ok")] = true;
  JsonObject pinObject = response.createNestedObject(F("gpio"));
  addPinState(pinObject, pin, state);
  sendJsonDocument(200, response);
}

void handleGpioWrite() {
  DynamicJsonDocument request(kJsonDocumentBytes);
  if (!parseJsonRequest(request)) {
    return;
  }

  const PinDefinition *pin = nullptr;
  size_t index = 0;
  if (!parsePin(request[F("pin")], pin, index)) {
    return;
  }

  performGpioWrite(*pin, index, request);
}

bool requireHttpMethod(HTTPMethod expected) {
  if (webServer->method() == expected) {
    return true;
  }

  sendJsonError(405, F("method_not_allowed"));
  return false;
}

void handlePathStyleGpioRoute() {
  const String prefix = F("/api/gpio/pin/");
  const String uri = webServer->uri();
  if (!uri.startsWith(prefix)) {
    sendJsonError(404, F("not_found"), uri);
    return;
  }

  const String remainder = uri.substring(prefix.length());
  const int separator = remainder.indexOf('/');
  if (separator <= 0 || separator >= static_cast<int>(remainder.length()) - 1) {
    sendJsonError(400, F("invalid_gpio_path"),
                  F("expected /api/gpio/pin/<pin>/<operation>"));
    return;
  }

  const String pinSegment = remainder.substring(0, separator);
  const String operation = remainder.substring(separator + 1);
  const int nestedSeparator = operation.indexOf('/');
  if (nestedSeparator >= 0 && operation != F("pwm/stop")) {
    sendJsonError(400, F("invalid_gpio_path"),
                  F("expected /api/gpio/pin/<pin>/<operation>"));
    return;
  }
  const PinDefinition *pin = nullptr;
  size_t index = 0;
  if (!parsePinFromPath(pinSegment, pin, index)) {
    return;
  }

  if (operation == F("read")) {
    if (!requireHttpMethod(HTTP_GET)) {
      return;
    }
    performGpioRead(*pin, index);
    return;
  }

  if (operation == F("adc")) {
    if (!requireHttpMethod(HTTP_GET)) {
      return;
    }
    performGpioAdcRead(*pin);
    return;
  }

  if (operation == F("configure")) {
    if (!requireHttpMethod(HTTP_POST)) {
      return;
    }
    DynamicJsonDocument request(kJsonDocumentBytes);
    if (!parseJsonRequest(request)) {
      return;
    }
    performGpioConfigure(*pin, index, request);
    return;
  }

  if (operation == F("write")) {
    if (!requireHttpMethod(HTTP_POST)) {
      return;
    }
    DynamicJsonDocument request(kJsonDocumentBytes);
    if (!parseJsonRequest(request)) {
      return;
    }
    performGpioWrite(*pin, index, request);
    return;
  }

  if (operation == F("dac")) {
    if (!requireHttpMethod(HTTP_POST)) {
      return;
    }
    DynamicJsonDocument request(kJsonDocumentBytes);
    if (!parseJsonRequest(request)) {
      return;
    }
    performGpioDacWrite(*pin, index, request);
    return;
  }

  if (operation == F("pwm")) {
    if (webServer->method() == HTTP_GET) {
      performGpioPwmRead(*pin, index);
      return;
    }

    if (!requireHttpMethod(HTTP_POST)) {
      return;
    }
    DynamicJsonDocument request(kJsonDocumentBytes);
    if (!parseJsonRequest(request)) {
      return;
    }
    performGpioPwmWrite(*pin, index, request);
    return;
  }

  if (operation == F("pwm/stop")) {
    if (!requireHttpMethod(HTTP_POST)) {
      return;
    }
    performGpioPwmStop(*pin, index);
    return;
  }

  sendJsonError(404, F("unknown_gpio_operation"), operation);
}

void configureRoutes() {
  webServer->on(F("/api/gpio"), HTTP_GET, handleGpioList);
  webServer->on(F("/api/gpio/read"), HTTP_GET, handleGpioRead);
  webServer->on(F("/api/gpio/adc"), HTTP_GET, handleGpioAdcRead);
  webServer->on(F("/api/gpio/configure"), HTTP_POST, handleGpioConfigure);
  webServer->on(F("/api/gpio/write"), HTTP_POST, handleGpioWrite);
  webServer->on(F("/api/gpio/dac"), HTTP_POST, handleGpioDacWrite);
  webServer->on(F("/api/gpio/pwm"), HTTP_GET, handleGpioPwmRead);
  webServer->on(F("/api/gpio/pwm"), HTTP_POST, handleGpioPwmWrite);
  webServer->on(F("/api/gpio/pwm/stop"), HTTP_POST, handleGpioPwmStop);
  webServer->onNotFound(handlePathStyleGpioRoute);
}

void resetRuntimeState() {
  for (size_t i = 0; i < sizeof(kPins) / sizeof(kPins[0]); ++i) {
    pinStates[i] = {false, F("unconfigured"), false, false, false, -1, -1,
                    false, -1, 0, 0, 0};
  }
  for (uint8_t channel = 0; channel < kMaxPwmChannels; ++channel) {
    pwmChannelInUse[channel] = false;
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
