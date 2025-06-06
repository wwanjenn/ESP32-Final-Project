#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Keypad.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>

// WiFi credentials
const char* ssid = "Wanjin Maglangit";
const char* password = "12345678910";

LiquidCrystal_I2C lcd(0x27, 16, 2);  // 16 columns, 2 rows

// XAMPP server settings
String HOST_NAME = "http://192.168.108.162"; // Change this!!
String PATH_NAME = "/collect_data.php";

// RFID
#define SS_PIN 5
#define RST_PIN 27
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {23, 22, 21, 19};
byte colPins[COLS] = {18, 17, 16, 4};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Servo
Servo myServo;
int servoPin = 13;

// Bluetooth
#define BTSerial Serial2
#define BT_RX 16
#define BT_TX 17

// Struct for valid users
struct User {
  String name;
  String password;
};

// List of authorized users
User users[] = {
  {"Alice", "A1B2C3D4"},
  {"Bob",   "1234"},
  {"Carol", "B7F8E9AA"},
  {"Admin", "9999"}
};

const int userCount = sizeof(users) / sizeof(users[0]);

void setup() {
  Serial.begin(115200);
  BTSerial.begin(9600, SERIAL_8N1, BT_RX, BT_TX);
  SPI.begin();
  mfrc522.PCD_Init();

  myServo.attach(servoPin);
  myServo.write(0);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready");

}

void loop() {
  checkRFID();
  checkKeypad();
  checkBluetooth();
}

void checkRFID() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String rfidData = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      rfidData += String(mfrc522.uid.uidByte[i], HEX);
    }
    rfidData.toUpperCase();
    Serial.println("RFID: " + rfidData);
    checkAccess("RFID", rfidData);
    mfrc522.PICC_HaltA();
  }
}

void checkKeypad() {
  static String inputBuffer = "";
  char key = keypad.getKey();
  if (key) {
    Serial.println("Keypad: " + String(key));
    
    if (key == '#') {
      checkAccess("Keypad", inputBuffer);
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Checking...");
      delay(1000); // short delay for readability
      lcd.clear();
      inputBuffer = "";
      lcd.setCursor(0,0);
      lcd.print("Enter Code:");
      
    } else if (key == '*') {
      inputBuffer = "";
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Cleared");
      delay(500);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Enter Code:");
      
    } else {
      if (inputBuffer.length() < 16) {
        inputBuffer += key;  // Limit input length
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Enter Code:");
        lcd.setCursor(0,1);
        lcd.print(inputBuffer);
      }
    }
  }
}

void checkBluetooth() {
  if (BTSerial.available()) {
    String btData = BTSerial.readStringUntil('\n');
    btData.trim();
    Serial.println("Bluetooth: " + btData);
    checkAccess("Bluetooth", btData);
  }
}


void activateServo() {
  myServo.write(90);
  delay(3000);
  myServo.write(0);
}

void checkAccess(String method, String input) {
  for (int i = 0; i < userCount; i++) {
    if (input == users[i].password) {
      Serial.println("Access Granted for " + users[i].name);
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Access Granted");
      lcd.setCursor(0,1);
      lcd.print(users[i].name);
      
      activateServo();
      logToServer(method, users[i].name, "Opened");
      delay(2000); // Show message for 2 seconds
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Enter Code:");
      return;
    }
  }
  Serial.println("Access Denied for input: " + input);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Access Denied");
  lcd.setCursor(0,1);
  lcd.print(input);
  
  logToServer(method, "Unknown", "Closed");
  delay(2000); // Show message for 2 seconds
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter Code:");
}


void logToServer(String method, String name, String status) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(HOST_NAME + PATH_NAME);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "method=" + method + "&name=" + name + "&status=" + status;
    int httpResponseCode = http.POST(httpRequestData);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server: " + response);
    } else {
      Serial.println("HTTP POST error: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected");
  }
}
