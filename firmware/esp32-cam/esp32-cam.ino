#include "esp_camera.h"

// AI-Thinker ESP32-CAM camera pins.
// These match the CAMERA_MODEL_AI_THINKER block in camerawebserversketch.
#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27

#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM    5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

// Direct PIR bring-up wiring:
// HC-SR501 OUT -> ESP32-CAM GPIO13
// HC-SR501 VCC -> ESP32-CAM 5V
// HC-SR501 GND -> ESP32-CAM GND
//
// GPIO13 is exposed on common AI-Thinker ESP32-CAM boards and is not used by
// the camera sensor. Do not use the microSD slot in this test because GPIO13 is
// one of the SD-card lines.
constexpr int PIR_PIN = 13;
constexpr int FLASH_LED_PIN = 4;

constexpr unsigned long PIR_STABLE_HIGH_MS = 250;
constexpr unsigned long EVENT_COOLDOWN_MS = 5000;
constexpr unsigned long PIR_WARMUP_MS = 30000;

bool cameraReady = false;
bool lastPirLevel = false;
bool highCandidateActive = false;
unsigned long highCandidateStartMs = 0;
unsigned long lastEventMs = 0;
unsigned long triggerCount = 0;
unsigned long ignoredCount = 0;
unsigned long captureCount = 0;

void blinkFlash(int count, int onMs, int offMs) {
  for (int i = 0; i < count; i++) {
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(onMs);
    digitalWrite(FLASH_LED_PIN, LOW);
    delay(offMs);
  }
}

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  if (!psramFound()) {
    Serial.println("PSRAM not found; using smaller frame buffer in DRAM.");
    config.frame_size = FRAMESIZE_QVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;
  }

  sensor_t *sensor = esp_camera_sensor_get();
  sensor->set_framesize(sensor, config.frame_size);
  Serial.println("Camera init OK.");
  return true;
}

bool captureTestFrame() {
  if (!cameraReady) {
    Serial.println("CAPTURE_SKIPPED: camera is not initialized.");
    return false;
  }

  camera_fb_t *frame = esp_camera_fb_get();
  if (!frame) {
    Serial.println("CAPTURE_FAIL: camera_fb_get returned null.");
    return false;
  }

  captureCount++;
  Serial.printf(
    "CAPTURE_OK: capture=%lu bytes=%u format=%u width=%u height=%u\n",
    captureCount,
    frame->len,
    frame->format,
    frame->width,
    frame->height
  );

  esp_camera_fb_return(frame);
  return true;
}

void handleMotionEvent(unsigned long nowMs) {
  triggerCount++;

  if (nowMs - lastEventMs < EVENT_COOLDOWN_MS) {
    ignoredCount++;
    Serial.printf(
      "PIR_IGNORED: trigger=%lu ignored=%lu reason=cooldown\n",
      triggerCount,
      ignoredCount
    );
    return;
  }

  lastEventMs = nowMs;
  Serial.printf("PIR_TRIGGERED: trigger=%lu pin=GPIO%d\n", triggerCount, PIR_PIN);
  blinkFlash(2, 120, 120);

  bool captured = captureTestFrame();
  Serial.printf(
    "EVENT_DONE: trigger=%lu captured=%s ignored=%lu total_captures=%lu\n",
    triggerCount,
    captured ? "true" : "false",
    ignoredCount,
    captureCount
  );
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);
  pinMode(PIR_PIN, INPUT_PULLDOWN);

  Serial.println();
  Serial.println("--- ESP32-CAM PIR Capture Bring-Up ---");
  Serial.println("Wiring: PIR VCC->5V, PIR GND->GND, PIR OUT->GPIO13.");
  Serial.println("Expected: GPIO13 LOW when idle, HIGH when motion is detected.");

  cameraReady = initCamera();
  if (!cameraReady) {
    Serial.println("Camera failed to initialize. Check AI-Thinker board selection and power.");
    blinkFlash(5, 80, 80);
  }

  Serial.printf("PIR warmup for %lu seconds...\n", PIR_WARMUP_MS / 1000);
}

void loop() {
  unsigned long nowMs = millis();
  bool pirLevel = digitalRead(PIR_PIN) == HIGH;

  if (nowMs < PIR_WARMUP_MS) {
    delay(50);
    return;
  }

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
    handleMotionEvent(nowMs);
  }

  lastPirLevel = pirLevel;
  delay(20);
}
