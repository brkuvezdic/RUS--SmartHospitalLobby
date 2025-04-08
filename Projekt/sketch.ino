#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD at 0x27, 16x2

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {12, 13, 14, 15}; // ESP32 pins for R1-R4
byte colPins[COLS] = {5, 18, 19, 23};  // ESP32 pins for C1-C4

void setup() {
  lcd.init();          // Initialize LCD
  lcd.backlight();     // Turn on backlight
  lcd.setCursor(0, 0); // Start at top-left
  lcd.print("Press a key:");
  
  // Initialize row pins as INPUT_PULLUP
  for (byte r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }
}

void loop() {
  char key = getKey(); // Check for keypress
  if (key) {           // If a key was pressed
    lcd.setCursor(0, 1); // Move to second row
    lcd.print("You pressed: ");
    lcd.print(key);
    delay(500); // Small delay to avoid repeated inputs
  }
}

// Custom keypad reading function (debounced)
char getKey() {
  for (byte c = 0; c < COLS; c++) {
    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], LOW);
    for (byte r = 0; r < ROWS; r++) {
      if (digitalRead(rowPins[r]) == LOW) {  // Fixed: Added closing parenthesis and comparison
        delay(50); // Debounce delay
        while (digitalRead(rowPins[r]) == LOW); // Wait for release
        pinMode(colPins[c], INPUT_PULLUP); // Reset column pin
        return keys[r][c];
      }
    }
    pinMode(colPins[c], INPUT_PULLUP); // Reset column pin
  }
  return 0; // No key pressed
}