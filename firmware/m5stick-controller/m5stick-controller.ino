#include <M5Unified.h>

// StickS3 pin definitions.
// Hat2 bus: G5 = PIR input, G7/G8 = camera UART.
constexpr int PIR_PIN = 5;
constexpr int CAMERA_TX_PIN = 7;
constexpr int CAMERA_RX_PIN = 8;
constexpr uint32_t CAMERA_BAUD = 115200;

// The current ESP32-CAM test sketch listens for 'P'.
// Change to "CAPTURE" once the camera firmware consumes that command.
constexpr const char *CAMERA_TRIGGER_COMMAND = "P";

HardwareSerial CameraSerial(1);

// State variables
volatile bool motionDetected = false;

// ISR runs in IRAM for maximum speed
void IRAM_ATTR motionISR() {
  motionDetected = true;
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  // StickS3 disables external 5V by default after M5.begin().
  // Enable it so the Hat2 EXT_5V rail can power the PIR and ESP32-CAM.
  M5.Power.setExtOutput(true);

  // Screen setup in landscape.
  M5.Lcd.setRotation(1); 
  M5.Lcd.fillScreen(BLACK);
  
  // Set text size to 2.
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("SYSTEM: ARMED");

  // Setup PIR Pin with Internal Pulldown
  pinMode(PIR_PIN, INPUT_PULLDOWN); 
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionISR, RISING);

  // Initialize USB serial for debugging and UART1 for the camera trigger.
  Serial.begin(115200);
  CameraSerial.begin(CAMERA_BAUD, SERIAL_8N1, CAMERA_RX_PIN, CAMERA_TX_PIN);
}

void loop() {
  if (motionDetected) {
    // 1. Send the Trigger to the Camera
    CameraSerial.println(CAMERA_TRIGGER_COMMAND);
    Serial.println("Motion detected; camera trigger sent.");

    // 2. High-Alert Visual
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setTextColor(WHITE);
    // drawCenterString(text, x, y, font_number)
    // Center using actual display dimensions so this works across Stick models.
    const int centerX = M5.Lcd.width() / 2;
    M5.Lcd.drawCenterString("MOTION", centerX, 45, 4);
    M5.Lcd.drawCenterString("DETECTED", centerX, 85, 4);

    // 3. Max Volume Alert
    // 4000Hz is a piercing "Smoke Alarm" style frequency
    M5.Speaker.tone(4000, 500); 

    // 4. Stay Red for 2 seconds
    delay(2000); 

    // 5. Reset to Idle
    motionDetected = false;
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.drawCenterString("SCANNING...", M5.Lcd.width() / 2, 80, 2);
  }
  
  M5.update(); 
}
