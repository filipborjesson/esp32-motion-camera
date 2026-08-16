#include <M5Unified.h>

// StickS3 Hat2 bus wiring.
// PIR OUT -> G5
// StickS3 G7/TX -> ESP32-CAM U0R/GPIO3
// StickS3 G8/RX -> ESP32-CAM U0T/GPIO1
constexpr int PIR_PIN = 5;
constexpr int CAMERA_TX_PIN = 7;
constexpr int CAMERA_RX_PIN = 8;
constexpr uint32_t CAMERA_BAUD = 115200;

constexpr unsigned long PIR_STABLE_HIGH_MS = 250;
constexpr unsigned long EVENT_COOLDOWN_MS = 5000;
constexpr unsigned long ALARM_BEEP_INTERVAL_MS = 450;
constexpr unsigned long SCREEN_FLASH_INTERVAL_MS = 350;
constexpr unsigned long STATUS_REFRESH_INTERVAL_MS = 500;
constexpr unsigned long CAMERA_RESPONSE_TIMEOUT_MS = 3000;

constexpr const char *DEVICE_ID = "stick-s3-01";
constexpr const char *CAPTURE_COMMAND = "CAPTURE";
constexpr const char *MOTION_MESSAGE = "MOTION:";
constexpr const char *PIR_TRIGGERED_MESSAGE = "PIR_TRIGGERED:";

HardwareSerial CameraSerial(1);

enum class SystemMode {
  Armed,
  AlarmTripped,
  Disarmed,
  Cooldown,
  CameraError
};

SystemMode mode = SystemMode::Armed;

bool lastPirLevel = false;
bool highCandidateActive = false;
bool flashOn = false;
bool waitingForCamera = false;
bool hasAcceptedEvent = false;

unsigned long highCandidateStartMs = 0;
unsigned long lastAcceptedEventMs = 0;
unsigned long alarmStartedMs = 0;
unsigned long lastBeepMs = 0;
unsigned long lastFlashMs = 0;
unsigned long lastStatusDrawMs = 0;
unsigned long cameraCommandSentMs = 0;

unsigned long triggerCount = 0;
unsigned long ignoredCount = 0;
unsigned long captureCommandCount = 0;
unsigned long cameraOkCount = 0;
unsigned long cameraFailCount = 0;

String cameraLine;
String lastAction = "Booting";

const char *modeName(SystemMode currentMode) {
  switch (currentMode) {
    case SystemMode::Armed:
      return "ARMED";
    case SystemMode::AlarmTripped:
      return "ALARM";
    case SystemMode::Disarmed:
      return "DISARMED";
    case SystemMode::Cooldown:
      return "COOLDOWN";
    case SystemMode::CameraError:
      return "CAM ERROR";
  }
  return "UNKNOWN";
}

uint16_t modeColor(SystemMode currentMode) {
  switch (currentMode) {
    case SystemMode::Armed:
      return DARKGREEN;
    case SystemMode::AlarmTripped:
      return flashOn ? YELLOW : ORANGE;
    case SystemMode::Disarmed:
      return DARKGREY;
    case SystemMode::Cooldown:
      return NAVY;
    case SystemMode::CameraError:
      return MAROON;
  }
  return BLACK;
}

void drawStatus(bool force = false) {
  unsigned long nowMs = millis();
  if (!force && nowMs - lastStatusDrawMs < STATUS_REFRESH_INTERVAL_MS) {
    return;
  }
  lastStatusDrawMs = nowMs;

  M5.Display.fillScreen(modeColor(mode));
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(8, 8);
  M5.Display.printf("Mode: %s\n", modeName(mode));

  M5.Display.setTextSize(1);
  M5.Display.setCursor(8, 42);
  M5.Display.printf("Device: %s\n", DEVICE_ID);
  M5.Display.printf("Triggers: %lu\n", triggerCount);
  M5.Display.printf("Ignored:  %lu\n", ignoredCount);
  M5.Display.printf("Capture commands: %lu\n", captureCommandCount);
  M5.Display.printf("Camera OK/Fail: %lu/%lu\n", cameraOkCount, cameraFailCount);
  M5.Display.printf("Last: %s\n", lastAction.c_str());

  M5.Display.setCursor(8, M5.Display.height() - 28);
  if (mode == SystemMode::AlarmTripped) {
    M5.Display.println("BtnA: disarm");
  } else if (mode == SystemMode::Disarmed) {
    M5.Display.println("BtnB: arm");
  } else {
    M5.Display.println("BtnA disarm | BtnB arm");
  }
}

void enterMode(SystemMode nextMode, const String &action) {
  mode = nextMode;
  lastAction = action;
  flashOn = false;

  if (mode == SystemMode::AlarmTripped) {
    alarmStartedMs = millis();
    lastBeepMs = 0;
    lastFlashMs = 0;
  }

  drawStatus(true);
  Serial.printf("MODE: %s action=%s\n", modeName(mode), lastAction.c_str());
}

void sendCameraCaptureCommand() {
  captureCommandCount++;
  waitingForCamera = true;
  cameraCommandSentMs = millis();
  cameraLine = "";

  CameraSerial.printf("%s:%lu\n", CAPTURE_COMMAND, triggerCount);
  Serial.printf("CAMERA_TX: %s:%lu\n", CAPTURE_COMMAND, triggerCount);
}

void handleAcceptedMotion(unsigned long nowMs) {
  triggerCount++;
  lastAcceptedEventMs = nowMs;
  hasAcceptedEvent = true;

  enterMode(SystemMode::AlarmTripped, "Motion accepted");
  sendCameraCaptureCommand();
}

void handleIgnoredMotion(const char *reason) {
  ignoredCount++;
  lastAction = String("Ignored: ") + reason;
  Serial.printf("PIR_IGNORED: ignored=%lu reason=%s\n", ignoredCount, reason);
  drawStatus(true);
}

void handleCameraMotionMessage(const String &message) {
  // The ESP32-CAM owns the PIR and image capture. A UART motion message is
  // therefore an event notification, not a request for another capture.
  triggerCount++;
  lastAcceptedEventMs = millis();
  hasAcceptedEvent = true;
  waitingForCamera = false;

  enterMode(SystemMode::AlarmTripped, message);
  Serial.printf("CAMERA_MOTION: trigger=%lu message=%s\n", triggerCount, message.c_str());
}

void pollPir() {
  if (mode == SystemMode::Disarmed) {
    lastPirLevel = digitalRead(PIR_PIN) == HIGH;
    highCandidateActive = false;
    return;
  }

  unsigned long nowMs = millis();
  bool pirLevel = digitalRead(PIR_PIN) == HIGH;

  if (pirLevel && !lastPirLevel) {
    highCandidateActive = true;
    highCandidateStartMs = nowMs;
    Serial.println("PIR_EDGE: rising edge candidate.");
  }

  if (!pirLevel) {
    highCandidateActive = false;
  }

  if (highCandidateActive && pirLevel && nowMs - highCandidateStartMs >= PIR_STABLE_HIGH_MS) {
    highCandidateActive = false;

    if (nowMs - lastAcceptedEventMs < EVENT_COOLDOWN_MS) {
      handleIgnoredMotion("cooldown");
    } else {
      handleAcceptedMotion(nowMs);
    }
  }

  lastPirLevel = pirLevel;
}

void pollCameraSerial() {
  while (CameraSerial.available()) {
    char ch = static_cast<char>(CameraSerial.read());
    if (ch == '\n' || ch == '\r') {
      cameraLine.trim();
      if (cameraLine.length() > 0) {
        Serial.printf("CAMERA_RX: %s\n", cameraLine.c_str());

        if (cameraLine.startsWith(MOTION_MESSAGE) || cameraLine.startsWith(PIR_TRIGGERED_MESSAGE)) {
          handleCameraMotionMessage(cameraLine);
        } else if (cameraLine.indexOf("CAPTURE_OK") >= 0 || cameraLine.indexOf("UPLOAD_OK") >= 0) {
          cameraOkCount++;
          waitingForCamera = false;
          lastAction = cameraLine;
          drawStatus(true);
        } else if (cameraLine.indexOf("FAIL") >= 0 || cameraLine.indexOf("ERROR") >= 0) {
          cameraFailCount++;
          waitingForCamera = false;
          enterMode(SystemMode::CameraError, cameraLine);
        } else {
          lastAction = cameraLine;
          drawStatus(true);
        }
      }
      cameraLine = "";
    } else if (cameraLine.length() < 96) {
      cameraLine += ch;
    }
  }

  if (waitingForCamera && millis() - cameraCommandSentMs > CAMERA_RESPONSE_TIMEOUT_MS) {
    waitingForCamera = false;
    cameraFailCount++;
    lastAction = "Camera timeout";
    Serial.println("CAMERA_TIMEOUT");
    drawStatus(true);
  }
}

void updateAlarmPresentation() {
  if (mode != SystemMode::AlarmTripped) {
    return;
  }

  unsigned long nowMs = millis();

  if (nowMs - lastFlashMs >= SCREEN_FLASH_INTERVAL_MS) {
    flashOn = !flashOn;
    lastFlashMs = nowMs;
    drawStatus(true);
  }

  if (nowMs - lastBeepMs >= ALARM_BEEP_INTERVAL_MS) {
    lastBeepMs = nowMs;
    const uint16_t alarmFrequency = flashOn ? 1600 : 2200;
    M5.Speaker.tone(alarmFrequency, 300);
  }
}

void updateCooldown() {
  if (mode == SystemMode::AlarmTripped || mode == SystemMode::CameraError) {
    return;
  }

  if (hasAcceptedEvent && mode == SystemMode::Armed && millis() - lastAcceptedEventMs < EVENT_COOLDOWN_MS) {
    mode = SystemMode::Cooldown;
    drawStatus(true);
  }

  if (hasAcceptedEvent && mode == SystemMode::Cooldown && millis() - lastAcceptedEventMs >= EVENT_COOLDOWN_MS) {
    enterMode(SystemMode::Armed, "Ready");
  }
}

void handleButtons() {
  if (M5.BtnA.wasPressed()) {
    waitingForCamera = false;
    enterMode(SystemMode::Disarmed, "Disarmed by BtnA");
    M5.Speaker.tone(260, 120);
  }

  if (M5.BtnB.wasPressed()) {
    waitingForCamera = false;
    lastAcceptedEventMs = 0;
    hasAcceptedEvent = false;
    enterMode(SystemMode::Armed, "Armed by BtnB");
    M5.Speaker.tone(880, 120);
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  // StickS3 disables external 5V by default after M5.begin().
  // Enable it so the Hat2 EXT_5V rail can power the PIR and ESP32-CAM.
  M5.Power.setExtOutput(true);

  M5.Display.setRotation(1);
  M5.Display.setTextSize(1);
  M5.Speaker.setVolume(255);

  Serial.begin(115200);
  CameraSerial.begin(CAMERA_BAUD, SERIAL_8N1, CAMERA_RX_PIN, CAMERA_TX_PIN);

  pinMode(PIR_PIN, INPUT_PULLDOWN);

  Serial.println();
  Serial.println("--- StickS3 Motion Assistant ---");
  Serial.println("PIR OUT -> G5, camera UART TX/RX -> G7/G8.");
  Serial.println("BtnA disarms. BtnB arms.");

  enterMode(SystemMode::Armed, "Ready");
}

void loop() {
  M5.update();

  handleButtons();
  pollPir();
  pollCameraSerial();
  updateAlarmPresentation();
  updateCooldown();
  drawStatus();

  delay(10);
}
