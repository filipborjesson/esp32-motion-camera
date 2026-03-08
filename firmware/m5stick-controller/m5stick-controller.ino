#include <M5StickCPlus2.h>

// Pin Definitions
const int PIR_PIN = 26; 

// State variables
volatile bool motionDetected = false;

// This function runs INSTANTLY when G26 goes HIGH
void IRAM_ATTR motionISR() {
  motionDetected = true;
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  // Setup Screen
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("PIR SCANNER READY");

  // Setup PIR Pin with Interrupt
  pinMode(PIR_PIN, INPUT_PULLDOWN); 
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionISR, RISING);

  Serial.begin(115200);
}

void loop() {
  if (motionDetected) {
    // 1. Notify Serial (Future Camera Trigger)
    Serial.println("CAPTURE");

    // 2. Visual Alert
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.drawCenterString("MOTION DETECTED", 120, 60, 4);

    // 3. Audio Alert (700Hz for 100ms)
    M5.Speaker.tone(700, 100);

    // 4. Hold state for a moment so we can see it
    delay(2000); 

    // Reset for next trigger
    motionDetected = false;
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.drawCenterString("SCANNING...", 120, 60, 4);
  }
  
  M5.update(); // Handle button presses etc
}