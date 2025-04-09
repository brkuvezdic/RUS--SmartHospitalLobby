#include <Wire.h>
#include <LiquidCrystal_I2C.h>

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

// Statički skup podataka: brojevi iskaznica i sobe
struct CardInfo {
  String cardNumber;
  String roomNumber;
};

CardInfo cardDatabase[] = {
  {"12345", "Soba 101"},
  {"67890", "Soba 102"},
  {"54321", "Soba 103"}
};

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Unesite broj:");

  // Inicijalizacija pinova za tipkovnicu
  for (byte r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }
}

void loop() {
  String cardNumber = getCardNumber();
  if (cardNumber.length() == 5) {
    CardInfo card = getCardInfo(cardNumber);
    if (card.cardNumber != "") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Iskaznica: ");
      lcd.print(card.cardNumber);
      lcd.setCursor(0, 1);
      lcd.print("Soba: ");
      lcd.print(card.roomNumber);
      delay(3000); // Pauza od 3 sekunde
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Nema u bazi");
      delay(3000); // Pauza od 3 sekunde
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unesite broj:");
  }
  delay(100); // Pauza za ponovni unos
}

// Funkcija za unos broja iskaznice s tipkovnice
String getCardNumber() {
  String cardNumber = "";
  while (cardNumber.length() < 5) {
    char key = getKey();
    if (key) {
      cardNumber += key;
      lcd.setCursor(cardNumber.length() - 1, 1);
      lcd.print(key);
    }
  }
  return cardNumber;
}

// Funkcija za dobivanje podataka o iskaznici
CardInfo getCardInfo(String cardNumber) {
  for (int i = 0; i < sizeof(cardDatabase) / sizeof(cardDatabase[0]); i++) {
    if (cardDatabase[i].cardNumber == cardNumber) {
      return cardDatabase[i]; // Vraća strukturu s podacima o iskaznici
    }
  }
  return {"", ""}; // Vraća prazne vrijednosti ako iskaznica nije pronađena
}

// Funkcija za provjeru postoji li broj iskaznice u statičkom skupu podataka
bool checkCard(String cardNumber) {
  for (int i = 0; i < sizeof(cardDatabase) / sizeof(cardDatabase[0]); i++) {
    if (cardDatabase[i].cardNumber == cardNumber) {
      return true;
    }
  }
  return false;
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
