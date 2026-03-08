#include <M5StickCPlus2.h>

// Pin Definitions
const int PIR_PIN = 26; 

// State variables
volatile bool motionDetected = false;

// ISR runs in IRAM for maximum speed
void IRAM_ATTR motionISR() {
  motionDetected = true;
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  // Screen Setup for Plus2 (Landscape)
  M5.Lcd.setRotation(1); 
  M5.Lcd.fillScreen(BLACK);
  
  // Set text size to 2 (Clean and readable on 240x320)
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("SYSTEM: ARMED");

  // Setup PIR Pin with Internal Pulldown
  pinMode(PIR_PIN, INPUT_PULLDOWN); 
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionISR, RISING);

  // Initialize Serial for Camera Trigger
  Serial.begin(115200);
}

void loop() {
  if (motionDetected) {
    // 1. Send the Trigger to the Camera
    Serial.println("CAPTURE");

    // 2. High-Alert Visual
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setTextColor(WHITE);
    // drawCenterString(text, x, y, font_number)
    // Font 4 is a decent "Impact" font that fits the Plus2 width
    M5.Lcd.drawCenterString("MOTION", 160, 60, 4);
    M5.Lcd.drawCenterString("DETECTED", 160, 100, 4);

    // 3. Max Volume Alert
    // 4000Hz is a piercing "Smoke Alarm" style frequency
    M5.Speaker.tone(4000, 500); 

    // 4. Stay Red for 2 seconds
    delay(2000); 

    // 5. Reset to Idle
    motionDetected = false;
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.drawCenterString("SCANNING...", 160, 100, 2);
  }
  
  M5.update(); 
}