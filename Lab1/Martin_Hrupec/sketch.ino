// biblioteka za koristenje osnovnih funkcija za rad s ESP32
#include <Arduino.h>
// biblioteka za softwerski timer
#include <Ticker.h>

#define LED_PIN 2
#define BUTTON1_PIN 18
#define BUTTON2_PIN 19
#define SENSOR_PIN 34  // Analogni ulaz

volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool timerTriggered = false;

Ticker ticker;

// ISR za gumb 1, izvodi se kad se stisne prva tipka i mijanja stanje button1Pressed
void IRAM_ATTR handleButton1Press() {
    button1Pressed = !button1Pressed;
}

// ISR za gumb 2, // ISR za gumb 1, izvodi se kad se stisne druga tipka i mijanja stanje button2Pressed
void IRAM_ATTR handleButton2Press() {
    button2Pressed = !button2Pressed;
}

// Timer ISR funkcija - poziva se svake sekunde i postavlja timerTriggered = true
// koristi se za očitavanje senzora i paljenje LED-a
void onTimer() {
    timerTriggered = true;
}

void setup() {
  // pokreće serijsku komunikaciju s računalom brzinom 115200 bps
    Serial.begin(115200);
    
    // LED se postavlja kao izlaz
    pinMode(LED_PIN, OUTPUT);
    // tipke se postavljaju ulazi s internim pull-up otpornicima
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);
    
    // detektira kada se pritisne prva tipka
    attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), handleButton1Press, FALLING);
    // detektira kada se pristisne druga tipka
    attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), handleButton2Press, FALLING);
    
    ticker.attach(1.0, onTimer); // Postavlja prekid svake sekunde
    
    Serial.println("ESP32 Interrupt System Initialized with Ticker");
}

void loop() {

  // ako je button1Pressed == true, pali LED i ispisuje poruku u Serial Monitor
    if (button1Pressed) {
        Serial.println("Button 1 Pressed! LED ON");
        digitalWrite(LED_PIN, HIGH);
        button1Pressed = false;
    }

  // ako je button2Pressed = true, pali LED 3 puta
    if (button2Pressed) {
        Serial.println("Button 2 Pressed! LED Blinking");
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(300);
            digitalWrite(LED_PIN, LOW);
            delay(300);
        }
        button2Pressed = false;
    }

  // kada je timerTriggered = true, ocitava se vrijednost analognog senzora i LED se prebacuje u suprptno stanje
    if (timerTriggered) {
        int sensorValue = analogRead(SENSOR_PIN);
        Serial.printf("Timer Interrupt: Sensor Value = %d\n", sensorValue);
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        timerTriggered = false;
    }
}