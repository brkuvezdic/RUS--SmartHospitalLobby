#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Postavka za simulirani WiFi način rada
bool simulatedWiFiMode = true;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD na 0x27, 16x2

// Tipkovnica
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {12, 13, 14, 15}; // Pinovi za redove tipkovnice
byte colPins[COLS] = {5, 18, 19, 23};  // Pinovi za stupce tipkovnice

// Pinovi za LED indikatore
const int ledSuccessPin = 2; // Zeleni LED (uspjeh)
const int ledErrorPin = 4;   // Crveni LED (greška)

// Struktura za pohranu podataka o pacijentu
struct PatientInfo {
  String cardNumber;
  String fullName;
  String roomNumber;
  String appointmentTime;
  String doctor;
  bool isValid;
};

// Stanja sustava
enum SystemState {
  WAITING_FOR_INPUT,
  PROCESSING_CARD,
  DISPLAY_PATIENT_INFO,
  DISPLAY_ERROR
};

const char* patientDataJson = R"rawliteral(
  [
    {"cardNumber":"12345", "fullName":"Ivan Horvat", "roomNumber":"101", "appointmentTime":"09:30", "doctor":"Dr. Perić"},
    {"cardNumber":"67890", "fullName":"Ana Kovač", "roomNumber":"102", "appointmentTime":"10:15", "doctor":"Dr. Marić"},
    {"cardNumber":"54321", "fullName":"Marko Novak", "roomNumber":"103", "appointmentTime":"11:00", "doctor":"Dr. Jurić"},
    {"cardNumber":"11111", "fullName":"Petra Kralj", "roomNumber":"104", "appointmentTime":"13:45", "doctor":"Dr. Kovačić"},
    {"cardNumber":"22222", "fullName":"Josip Tomić", "roomNumber":"105", "appointmentTime":"14:30", "doctor":"Dr. Babić"}
  ]
  )rawliteral";

SystemState currentState = WAITING_FOR_INPUT;
String inputBuffer = "";
unsigned long lastActionTime = 0;
const unsigned long TIMEOUT = 30000; // 30 sekundi timeout

void setup() {
  Serial.begin(115200);
  
  // Inicijalizacija LCD-a
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inicijalizacija");
  lcd.setCursor(0, 1);
  lcd.print("sustava...");

  // Inicijalizacija pinova za tipkovnicu
  for (byte r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }
  
  // Inicijalizacija LED pinova
  pinMode(ledSuccessPin, OUTPUT);
  pinMode(ledErrorPin, OUTPUT);
  
  // Simulirano spajanje na WiFi
  connectToWiFi();
  
  // Postavi početni ekran
  resetToInputMode();
}

void loop() {
  // Provjeri timeout
  if (millis() - lastActionTime > TIMEOUT) {
    resetToInputMode();
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
      // Ova stanja samo čekaju na timeout ili posebne tipke
      char key = getKey();
      if (key == 'D') { // D tipka za povratak
        resetToInputMode();
      }
      break;
  }
  
  delay(100);
}

void connectToWiFi() {
  if (simulatedWiFiMode) {
    // Simuliraj uspješno povezivanje
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi simuliran");
    lcd.setCursor(0, 1);
    lcd.print("192.168.1.100");
    
    Serial.println("WiFi simuliran");
    Serial.println("Simulirana IP adresa: 192.168.1.100");
    
    delay(2000);
    return;
  }
  
  // Ovo se neće izvršiti u simuliranom načinu rada
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Spajanje na");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");
  
  WiFi.begin("SSID", "PASSWORD");
  
  int attemptCount = 0;
  while (WiFi.status() != WL_CONNECTED && attemptCount < 20) {
    delay(500);
    Serial.print(".");
    attemptCount++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi spojen");
    Serial.println("IP adresa: ");
    Serial.println(WiFi.localIP());
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi spojen");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
  } else {
    Serial.println("WiFi spajanje neuspješno");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi greška!");
    lcd.setCursor(0, 1);
    lcd.print("Offline način");
    delay(2000);
  }
}

void resetToInputMode() {
  inputBuffer = "";
  currentState = WAITING_FOR_INPUT;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Unesite broj:");
  lcd.setCursor(0, 1);
  lastActionTime = millis();
  
  // Isključi LED indikatore
  digitalWrite(ledSuccessPin, LOW);
  digitalWrite(ledErrorPin, LOW);
}

void handleCardInput() {
  char key = getKey();
  if (key) {
    lastActionTime = millis();
    
    if (key == '#') {
      // Potvrdi unos
      if (inputBuffer.length() > 0) {
        currentState = PROCESSING_CARD;
      }
    } 
    else if (key == '*') {
      // Obriši zadnji znak
      if (inputBuffer.length() > 0) {
        inputBuffer = inputBuffer.substring(0, inputBuffer.length() - 1);
        lcd.setCursor(0, 1);
        lcd.print("                "); // Očisti red
        lcd.setCursor(0, 1);
        lcd.print(inputBuffer);
      }
    }
    else if (key >= '0' && key <= '9' && inputBuffer.length() < 16) {
      // Dodaj brojku
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
  
  PatientInfo patient = getPatientFromAPI(inputBuffer);
  
  if (patient.isValid) {
    displayPatientInfo(patient);
    currentState = DISPLAY_PATIENT_INFO;
    // Uspješna prijava - zeleni LED
    digitalWrite(ledSuccessPin, HIGH);
    digitalWrite(ledErrorPin, LOW);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Greška:");
    lcd.setCursor(0, 1);
    lcd.print("Nema u bazi");
    currentState = DISPLAY_ERROR;
    // Neuspješna prijava - crveni LED
    digitalWrite(ledSuccessPin, LOW);
    digitalWrite(ledErrorPin, HIGH);
  }
  
  lastActionTime = millis();
}

PatientInfo getPatientFromAPI(String cardNumber) {
  PatientInfo patient;
  patient.isValid = false;

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, patientDataJson);
  if (error) return patient;

  for (JsonObject obj : doc.as<JsonArray>()) {
    if (obj["cardNumber"] == cardNumber) {
      patient.cardNumber = obj["cardNumber"].as<String>();
      patient.fullName = obj["fullName"].as<String>();
      patient.roomNumber = obj["roomNumber"].as<String>();
      patient.appointmentTime = obj["appointmentTime"].as<String>();
      patient.doctor = obj["doctor"].as<String>();
      patient.isValid = true;
      break;
    }
  }

  delay(800);
  return patient;
}

void displayPatientInfo(PatientInfo patient) {
  // Prvi ekran - osnovne informacije
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ime: " + truncateString(patient.fullName, 11));
  lcd.setCursor(0, 1);
  lcd.print("Soba: " + patient.roomNumber);
  delay(3000);
  
  // Drugi ekran - dodatne informacije
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Termin: " + truncateString(patient.appointmentTime, 9));
  lcd.setCursor(0, 1);
  lcd.print("Dr: " + truncateString(patient.doctor, 13));
  
  // Prikaži uputu za povratak
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

// Funkcija za čitanje tipke s tipkovnice
char getKey() {
  for (byte c = 0; c < COLS; c++) {
    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], LOW);
    for (byte r = 0; r < ROWS; r++) {
      if (digitalRead(rowPins[r]) == LOW) {
        delay(50); // Debounce
        while (digitalRead(rowPins[r]) == LOW); // Čekaj dok ne otpusti
        pinMode(colPins[c], INPUT_PULLUP); // Resetiraj pin
        return keys[r][c];
      }
    }
    pinMode(colPins[c], INPUT_PULLUP); // Resetiraj pin
  }
  return 0; // Nema pritisnute tipke
}
