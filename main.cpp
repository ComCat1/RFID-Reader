#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RST_PIN         9
#define SS_PIN          10
#define OLED_RESET      -1
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MFRC522 mfrc522(SS_PIN, RST_PIN);

unsigned int tagCount = 0;
String lastUID = "";

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Initializing...");
  display.display();
  delay(2000); // Show initializing message for 2 seconds
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  String uidStr = getUIDString();
  if (uidStr != lastUID) {
    lastUID = uidStr;
    tagCount++;
    
    displayMultiLayeredScanningAnimation(); // Play the improved scanning animation
    updateDisplay(uidStr);
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void displayMultiLayeredScanningAnimation() {
  const int centerX = SCREEN_WIDTH / 2;
  const int centerY = SCREEN_HEIGHT / 2;
  const int maxRadius = sqrt(centerX * centerX + centerY * centerY); // To cover entire display
  const int steps = 120; // For smooth full rotation
  const int circleSteps = 20; // Number of steps before a circle completes its expansion
  const int delayTime = 20; // Milliseconds for each step
  
  for (int i = 0; i < steps; i++) {
    display.clearDisplay();
    
    // Draw expanding circles
    for (int j = 0; j < maxRadius; j += maxRadius / circleSteps) {
      int currentRadius = (i * maxRadius / steps + j) % maxRadius;
      int brightness = max(1, 255 - (currentRadius * 255 / maxRadius)); // Fading effect
      display.drawCircle(centerX, centerY, currentRadius, SSD1306_WHITE);
    }

    // Draw sweeping radial line
    float angle = radians((float)i / steps * 360);
    int endX = centerX + maxRadius * cos(angle);
    int endY = centerY + maxRadius * sin(angle);
    display.drawLine(centerX, centerY, endX, endY, SSD1306_WHITE);

    display.display();
    delay(delayTime);
  }

  // Clear the display to prepare for showing the scanned data
  display.clearDisplay();
}



String getUIDString() {
  String uidStr = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidStr += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") + String(mfrc522.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();
  return uidStr;
}



void updateDisplay(const String& uid) {
  display.clearDisplay(); // Clear the display for fresh update

  // Display "RFID UID" as a title
  display.setTextSize(1); // Normal 1:1 pixel scale
  display.setCursor(0,0);
  display.println("RFID UID");

  // Draw a horizontal line to separate title from content
  display.drawFastHLine(0, 9, SCREEN_WIDTH, SSD1306_WHITE);

  // Check if UID needs scrolling or pagination
  // For simplicity, let's assume it fits the display width for now
  display.setTextSize(1); // Choose appropriate text size
  display.setCursor(0, 12);
  display.println(uid);

  // Display the tag count at a fixed position to avoid overlap
  display.setTextSize(1);
  display.setCursor(0, SCREEN_HEIGHT - 8); // Adjust position as needed
  display.print("Count: ");
  display.println(tagCount);

  display.display(); // Refresh the display with new data
}


void displayError(const String& message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Error:");
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println(message);
  display.display();
}


