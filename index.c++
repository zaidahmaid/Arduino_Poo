#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Wire.h>

bool getFingerprintIDez();
void buzzerAlert(bool correctCard);

SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int x = 0;

#define SS_PIN 10
#define RST_PIN 9
String UID = "3A197D80";  // Removed spaces from the UID
byte lock = 0;

Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(SS_PIN, RST_PIN);
int buzzerPin = 6;  // Adjust the pin based on your wiring

void setup() {
  Serial.begin(9600);
  servo.attach(5);  // Initialize the servo
  lcd.init();
  lcd.backlight();
  SPI.begin();
  rfid.PCD_Init();
  finger.begin(57600);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Welcome!");
  lcd.setCursor(1, 1);
  lcd.print("Put your card");

  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wrong card!");
    buzzerAlert(false);  // Wrong card alert
    delay(1500);
    lcd.clear();
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scanning");
  Serial.print("NUID tag is: ");
  String ID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    lcd.print(".");
    ID.concat(String(rfid.uid.uidByte[i] < 0x10 ? "0" : ""));
    ID.concat(String(rfid.uid.uidByte[i], HEX));
    delay(300);
  }
  ID.toUpperCase();

  if(ID != UID) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wrong card!");
    buzzerAlert(false);  // Wrong card alert
    delay(1500);
    lcd.clear();
  } else if (ID == UID && lock == 0 && getFingerprintIDez()) {
    servo.write(90);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door is open");
    buzzerAlert(true);  // Correct card alert
    delay(1500);
    lcd.clear();
    lock = 1;
  } else if (ID == UID && lock == 1) {
    servo.write(200);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door is locked");
    buzzerAlert(true);  // Correct card alert
    delay(1500);
    lcd.clear();
    lock = 0;
  } 
}

bool getFingerprintIDez() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Place your ");
  lcd.setCursor(4, 1);
  lcd.print("finger...");

  while (true) {
    uint8_t p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      p = finger.image2Tz();
      if (p == FINGERPRINT_OK) {
        p = finger.fingerFastSearch();
        if (p == FINGERPRINT_OK) {
          lcd.clear();
          lcd.setCursor(2, 0);
          lcd.print("Correct");
          lcd.setCursor(2, 1);
          lcd.print("fingerPrint");
          buzzerAlert(true);  // Fingerprint found alert
          delay(1500);  // Optional delay for display
          return true;
        } else {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Wrong");
          lcd.setCursor(2, 1);
          lcd.print("fingerPrint!");
          buzzerAlert(false);  // Wrong fingerprint alert
          delay(1500);
          return false;
        }
      }
    }

    delay(500);  // Add a delay to avoid overwhelming the fingerprint sensor
  }
  return false;  // This should not be reached
}

void buzzerAlert(bool correctCard) {
  if (correctCard) {
    tone(buzzerPin, 1500, 500);  // Correct card or fingerprint alert
  } else {
    tone(buzzerPin, 500, 1000);  // Wrong card or fingerprint alert
  }
  delay(500);  // Add a delay between alerts
  noTone(buzzerPin);  // Turn off the buzzer
}
