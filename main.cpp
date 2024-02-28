#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RST_PIN             9
#define SS_PIN             10
#define BUTTON_READ_PIN     8
#define BUTTON_DISPLAY_PIN  7
#define SCREEN_WIDTH      128
#define SCREEN_HEIGHT      32
#define OLED_RESET        -1
#define DEBOUNCE_DELAY    200

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MFRC522 mfrc522(SS_PIN, RST_PIN);
String storedUIDs[5];
int currentUIDIndex = -1;
int menuIndex = 0; // Index for menu navigation

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(BUTTON_READ_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DISPLAY_PIN, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  welcomeMessage();
}

void loop() {
  showMenu();
  debounceAndOperate();
}

void debounceAndOperate() {
  static unsigned long lastDebounceTime = 0;
  static bool lastButtonState = LOW;
  bool buttonState = digitalRead(BUTTON_READ_PIN);

  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (buttonState == HIGH) {
      if (menuIndex == 0) {
        scanRFID();
      } else if (menuIndex == 1) {
        displayNextUID();
      }
    }
  }
  lastButtonState = buttonState;

  // Cycle through menu options
  if (digitalRead(BUTTON_DISPLAY_PIN) == HIGH) {
    menuIndex = (menuIndex + 1) % 2; // Increase menuIndex to cycle through options
    delay(DEBOUNCE_DELAY); // Simple delay for button debounce
    showMenu();
  }
}

void scanRFID() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Scanning...");
  display.display();
  
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    displayMessage("No Card Detected");
    return;
  }

  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") + String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  if (currentUIDIndex < 4) {
    storedUIDs[++currentUIDIndex] = uid;
    displayMessage("Card Stored: " + uid);
  } else {
    displayMessage("Memory Full");
  }
}

void displayNextUID() {
  if (currentUIDIndex == -1) {
    displayMessage("No Cards Stored");
    return;
  }

  static int displayIndex = 0;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("UID: ");
  display.setCursor(0, 10);
  display.print(storedUIDs[displayIndex]);
  display.display();

  displayIndex = (displayIndex + 1) % (currentUIDIndex + 1);
}

void displayMessage(String message) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(message);
  display.display();
  delay(2000);
}

void welcomeMessage() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("RFID Reader Ready");
  display.display();
  delay(2000);
}

void showMenu() {
  display.clearDisplay();
  display.setCursor(0, 0);
  if (menuIndex == 0) {
    display.print(">Scan Card");
    display.setCursor(0, 10);
    display.print(" View UIDs");
  } else if (menuIndex == 1) {
    display.print(" Scan Card");
    display.setCursor(0, 10);
    display.print(">View UIDs");
  }
  display.display();
}


