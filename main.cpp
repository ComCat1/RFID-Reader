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
  const int barWidth = SCREEN_WIDTH - 20; // Total width of the loading bar, with padding
  const int barHeight = 8; // Height of the loading bar
  const int startX = (SCREEN_WIDTH - barWidth) / 2; // Starting X position (centered)
  const int startY = (SCREEN_HEIGHT - barHeight) / 2; // Starting Y position (centered)
  const int steps = 120; // Number of steps to fill the loading bar
  const int delayTime = 20; // Milliseconds for each step
  
  for (int i = 0; i <= steps; i++) {
    int fillWidth = (i * barWidth) / steps; // Calculate current width to fill

    display.clearDisplay();
    // Draw the border of the loading bar
    display.drawRect(startX - 1, startY - 1, barWidth + 2, barHeight + 2, SSD1306_WHITE);
    // Fill the loading bar based on the current step
    display.fillRect(startX, startY, fillWidth, barHeight, SSD1306_WHITE);

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


