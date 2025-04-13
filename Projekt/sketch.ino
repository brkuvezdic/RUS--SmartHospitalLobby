#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ArduinoJson.h>

// Firebase configuration
#define FIREBASE_HOST "rus---projekt-default-rtdb.europe-west1.firebasedatabase.app"
#define FIREBASE_AUTH "Fg4FnkVhu41qNTYb4XjFI9VVeBmYBg9oYOblnfZJ"

// WiFi configuration
#define WIFI_SSID "WIFI"
#define WIFI_PASSWORD "WIFI_PASS"

// Simulated WiFi mode
bool simulatedWiFiMode = true;
bool hasInternet = false;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {12, 13, 14, 15};
byte colPins[COLS] = {5, 18, 19, 23};

// LED pins
const int ledSuccessPin = 2;
const int ledErrorPin = 4;

// Firebase objects
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

// Patient information structure
struct PatientInfo {
  String cardNumber;
  String fullName;
  String roomNumber;
  String appointmentTime;
  String doctor;
  bool isValid;
};

// Local database for offline mode
struct LocalPatient {
  const char* cardNumber;
  const char* fullName;
  const char* roomNumber;
  const char* appointmentTime;
  const char* doctor;
};

LocalPatient localPatients[] = {
  {"12345", "Ivan Horvat", "101", "09:30", "Dr. Perić"},
  {"67890", "Ana Kovač", "102", "10:15", "Dr. Marić"},
  {"54321", "Marko Novak", "103", "11:00", "Dr. Jurić"},
  {"11111", "Petra Kralj", "104", "13:45", "Dr. Kovačić"},
  {"22222", "Josip Tomić", "105", "14:30", "Dr. Babić"}
};

enum SystemState {
  WAITING_FOR_INPUT,
  PROCESSING_CARD,
  DISPLAY_PATIENT_INFO,
  DISPLAY_ERROR
};

SystemState currentState = WAITING_FOR_INPUT;
String inputBuffer = "";
unsigned long lastActionTime = 0;
const unsigned long TIMEOUT = 30000;

// Function prototypes - add these to fix the "not declared in this scope" errors
void connectToWiFi();
void resetToInputMode();
void handleCardInput();
void processCardNumber();
char getKey();
PatientInfo getPatientData(String cardNumber);
void displayPatientInfo(PatientInfo patient);
String truncateString(String str, int maxLength);

void setup() {
  Serial.begin(115200);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inicijalizacija");
  lcd.setCursor(0, 1);
  lcd.print("sustava...");

  // Initialize keypad pins
  for (byte r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }
  
  // Initialize LED pins
  pinMode(ledSuccessPin, OUTPUT);
  pinMode(ledErrorPin, OUTPUT);
  
  // Connect to WiFi
  connectToWiFi();
  
  // Initialize Firebase if connected
  if (hasInternet) {
    firebaseConfig.host = FIREBASE_HOST;
    firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&firebaseConfig, &firebaseAuth);
    Firebase.reconnectWiFi(true);
    Firebase.setReadTimeout(firebaseData, 1000 * 60);
    Firebase.setwriteSizeLimit(firebaseData, "tiny");
    Serial.println("Firebase initialized");
  }
  
  // Set initial screen
  resetToInputMode();
}

void loop() {
  // Check timeout
  if (millis() - lastActionTime > TIMEOUT) {
    resetToInputMode();
  }
  
  // Periodic WiFi check (every 30 seconds)
  static unsigned long lastWiFiCheck = 0;
  if (millis() - lastWiFiCheck > 30000) {
    connectToWiFi();
    lastWiFiCheck = millis();
  }
  
  switch (currentState) {
    case WAITING_FOR_INPUT:
      handleCardInput();
      break;
      
    case PROCESSING_CARD:
      processCardNumber();
      break;
      
    case DISPLAY_PATIENT_INFO:
    case DISPLAY_ERROR:
      // These states just wait for timeout or special keys
      char key = getKey();
      if (key == 'D') { // D key to return
        resetToInputMode();
      }
      break;
  }
  
  delay(100);
}

void connectToWiFi() {
  if (simulatedWiFiMode) {
    // Simulate WiFi in Wokwi environment
    WiFi.begin("Wokwi-GUEST", "", 6);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Simulirani WiFi");
    
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 10) {
      delay(500);
      lcd.print(".");
      attempt++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      hasInternet = true;
      lcd.setCursor(0, 1);
      lcd.print("IP: 192.168.1.100");
      Serial.println("Simulated WiFi connected");
    } else {
      hasInternet = false;
      lcd.setCursor(0, 1);
      lcd.print("Offline mod");
      Serial.println("Simulated WiFi not available");
    }
    delay(2000);
    return;
  }
  
  // Real WiFi connection
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Spajanje na WiFi");
  lcd.setCursor(0, 1);
  lcd.print(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attemptCount = 0;
  while (WiFi.status() != WL_CONNECTED && attemptCount < 10) {
    delay(500);
    Serial.print(".");
    attemptCount++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    hasInternet = true;
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi spojen");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
  } else {
    hasInternet = false;
    Serial.println("WiFi connection failed");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi greška!");
    lcd.setCursor(0, 1);
    lcd.print("Offline način");
  }
  delay(2000);
}

void resetToInputMode() {
  inputBuffer = "";
  currentState = WAITING_FOR_INPUT;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Unesite broj:");
  lcd.setCursor(0, 1);
  lastActionTime = millis();
  
  // Turn off LED indicators
  digitalWrite(ledSuccessPin, LOW);
  digitalWrite(ledErrorPin, LOW);
}

void handleCardInput() {
  char key = getKey();
  if (key) {
    lastActionTime = millis();
    
    if (key == '#') {
      // Confirm input
      if (inputBuffer.length() > 0) {
        currentState = PROCESSING_CARD;
      }
    } 
    else if (key == '*') {
      // Delete last character
      if (inputBuffer.length() > 0) {
        inputBuffer = inputBuffer.substring(0, inputBuffer.length() - 1);
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.print(inputBuffer);
      }
    }
    else if (key >= '0' && key <= '9' && inputBuffer.length() < 16) {
      // Add digit
      inputBuffer += key;
      lcd.setCursor(0, 1);
      lcd.print(inputBuffer);
    }
  }
}

void processCardNumber() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Provjera...");
  
  PatientInfo patient = getPatientData(inputBuffer);
  
  if (patient.isValid) {
    displayPatientInfo(patient);
    currentState = DISPLAY_PATIENT_INFO;
    // Successful login - green LED
    digitalWrite(ledSuccessPin, HIGH);
    digitalWrite(ledErrorPin, LOW);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Greška:");
    lcd.setCursor(0, 1);
    lcd.print("Nema u bazi");
    currentState = DISPLAY_ERROR;
    // Failed login - red LED
    digitalWrite(ledSuccessPin, LOW);
    digitalWrite(ledErrorPin, HIGH);
  }
  
  lastActionTime = millis();
}

PatientInfo getPatientData(String cardNumber) {
  PatientInfo patient;
  patient.isValid = false;

  // First try to get from Firebase
  if (hasInternet) {
    String path = "/pacijenti/" + cardNumber;
    
    if (Firebase.getJSON(firebaseData, path)) {
      String json = firebaseData.jsonString();

      if (json.isEmpty()) {
        Serial.println("Empty response from Firebase");
        return patient;
      }

      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, json);
      
      if (!error) {
        // Extract patient data
        patient.cardNumber = cardNumber;
        patient.fullName = doc["fullName"].as<String>();
        patient.roomNumber = doc["roomNumber"].as<String>();
        patient.appointmentTime = doc["appointmentTime"].as<String>();
        patient.doctor = doc["doctor"].as<String>();
        patient.isValid = true;
        Serial.println("Data retrieved from Firebase");
        return patient;
      } else {
        Serial.println("JSON deserialization failed: " + String(error.c_str()));
      }
    } else {
      Serial.println("Firebase response error: " + firebaseData.errorReason());
    }
  }

  // If Firebase is not available or data doesn't exist, try local database
  for (int i = 0; i < sizeof(localPatients) / sizeof(localPatients[0]); i++) {
    if (cardNumber == localPatients[i].cardNumber) {
      patient.cardNumber = cardNumber;
      patient.fullName = localPatients[i].fullName;
      patient.roomNumber = localPatients[i].roomNumber;
      patient.appointmentTime = localPatients[i].appointmentTime;
      patient.doctor = localPatients[i].doctor;
      patient.isValid = true;
      Serial.println("Data retrieved from local database");
      break;
    }
  }
  
  return patient;
}

void displayPatientInfo(PatientInfo patient) {
  // First screen - basic info
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ime: ");
  lcd.print(truncateString(patient.fullName, 11));
  lcd.setCursor(0, 1);
  lcd.print("Soba: ");
  lcd.print(patient.roomNumber);
  delay(3000);
  
  // Second screen - additional info
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Termin: ");
  lcd.print(truncateString(patient.appointmentTime, 8));
  lcd.setCursor(0, 1);
  lcd.print("Dr: ");
  lcd.print(truncateString(patient.doctor, 12));
  
  // Show return instruction
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pritisnite 'D'");
  lcd.setCursor(0, 1);
  lcd.print("za povratak");
}

String truncateString(String str, int maxLength) {
  if (str.length() <= maxLength) {
    return str;
  }
  return str.substring(0, maxLength);
}

char getKey() {
  for (byte c = 0; c < COLS; c++) {
    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], LOW);
    for (byte r = 0; r < ROWS; r++) {
      if (digitalRead(rowPins[r]) == LOW) {
        delay(50); // Debounce
        while (digitalRead(rowPins[r]) == LOW); // Wait for release
        pinMode(colPins[c], INPUT_PULLUP); // Reset pin
        return keys[r][c];
      }
    }
    pinMode(colPins[c], INPUT_PULLUP); // Reset pin
  }
  return 0; // No key pressed
}