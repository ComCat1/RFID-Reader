#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RST_PIN            9
#define SS_PIN            10
#define BUTTON_READ_PIN    8
#define BUTTON_DISPLAY_PIN 7
#define SCREEN_WIDTH     128
#define SCREEN_HEIGHT     32
#define OLED_RESET       -1
#define DEBOUNCE_DELAY   200

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MFRC522 mfrc522(SS_PIN, RST_PIN);
String storedUIDs[5]; // Memory for up to 5 card UIDs
int currentUIDIndex = -1; // Index for the currently displayed UID

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(BUTTON_READ_PIN, INPUT);
  pinMode(BUTTON_DISPLAY_PIN, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);
  welcomeMessage();
}

void loop() {
  debounceAndOperate();
}

void debounceAndOperate() {
  static unsigned long lastDebounceTime = 0;
  static bool lastReadState = LOW;
  static bool lastDisplayState = LOW;
  bool readState = digitalRead(BUTTON_READ_PIN);
  bool displayState = digitalRead(BUTTON_DISPLAY_PIN);
  unsigned long currentMillis = millis();

  if (readState != lastReadState) {
    lastDebounceTime = currentMillis;
  }
  if (displayState != lastDisplayState) {
    lastDebounceTime = currentMillis;
  }

  if ((currentMillis - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (readState == HIGH && lastReadState == LOW) {
      scanRFID();
    }
    if (displayState == HIGH && lastDisplayState == LOW) {
      displayNextUID();
    }
  }

  lastReadState = readState;
  lastDisplayState = displayState;
}

void scanRFID() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    displayMessage("No Card Detected");
    return;
  }
  
  // Check for potentially encrypted card based on SAK
  if (isEncryptedCard(mfrc522.uid.sak)) {
    displayMessage("Encrypted Card Detected");
    // Placeholder for further process
  } else {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") + String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    if (currentUIDIndex < 4) { // Adjust based on array size
      storedUIDs[++currentUIDIndex] = uid;
      displayMessage("Card Stored");
    } else {
      displayMessage("Memory Full");
    }
  }
}

bool isEncryptedCard(byte sak) {
  // Placeholder condition for identifying encrypted cards
  // This condition should be adjusted based on the specific card types you're dealing with
  return sak != 0x08; // Example SAK value for a potentially encrypted card
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
  
  displayIndex = (displayIndex + 1) % (currentUIDIndex + 1); // Cycle through stored UIDs
}

void displayMessage(String message) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(message);
  display.display();
  delay(2000); // Display message for 2 seconds
}

void welcomeMessage() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("RFID Reader Ready");
  display.display();
  delay(2000); // Show welcome message for 2 seconds
}

