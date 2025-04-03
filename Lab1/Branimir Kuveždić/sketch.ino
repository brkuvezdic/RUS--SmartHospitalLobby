#include <TimerOne.h>  // Uključivanje biblioteke za rad s timerom

// Definicije pinova
const int pirPin = 2;           // PIR senzor
const int ldrPin = A0;          // LDR senzor
const int buzzerPin = 3;        // Buzzer
const int ledPin = 4;           // LED dioda
const int btnDisablePin = 5;    // Tipka za onemogućavanje alarma
const int btnEnablePin = 6;     // Tipka za omogućavanje alarma

// Volatilne varijable koje se koriste u prekidnim rutinama
volatile bool motionDetected = false;
volatile bool alarmDisabled = false;
volatile bool timerTriggered = false;

void setup() {
  Serial.begin(9600);
  
  // Postavljanje modova pinova
  pinMode(pirPin, INPUT);
  pinMode(ldrPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(btnDisablePin, INPUT_PULLUP);
  pinMode(btnEnablePin, INPUT_PULLUP);
  
  // Postavljanje prekida
  attachInterrupt(digitalPinToInterrupt(pirPin), motionISR, RISING);
  attachInterrupt(digitalPinToInterrupt(btnDisablePin), disableAlarmISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(btnEnablePin), enableAlarmISR, FALLING);
  
  // Inicijalizacija TimerOne da se prekid poziva svakih 1 sekundu
  Timer1.initialize(1000000);  // 1.000.000 mikrosekundi = 1 sekunda
  Timer1.attachInterrupt(timerISR);
}

void loop() {
  // Čitanje svjetlosnog nivoa s LDR-a
  int lightLevel = analogRead(ldrPin);
  Serial.print("Light Level: ");
  Serial.println(lightLevel);
  
  // Ako je detektirana pokret i alarm nije onemogućen, provjeri uvjet za aktivaciju alarma
  if (motionDetected && !alarmDisabled) {
    if (lightLevel < 500) { // Ako je mrak, aktiviraj alarm
      Serial.println("Motion detected! Activating alarm...");
      digitalWrite(ledPin, HIGH);
      tone(buzzerPin, 1000);
      delay(3000);
      noTone(buzzerPin);
      digitalWrite(ledPin, LOW);
    }
    motionDetected = false;
  }
  
  // Obrada timer prekida – ovdje kao primjer, LED blink kad je alarm onemogućen
  if (timerTriggered) {
    timerTriggered = false;
    if (alarmDisabled) {
      // Ako je alarm onemogućen, blinkamo LED-om kako bi signalizirali status
      digitalWrite(ledPin, !digitalRead(ledPin));
    } else {
      // Inače osiguraj da je LED ugašen
      digitalWrite(ledPin, LOW);
    }
  }
  
  delay(500);
}

// Prekidna rutina za PIR senzor
void motionISR() {
  motionDetected = true;
}

// Prekidna rutina za tipku koja onemogućuje alarm
void disableAlarmISR() {
  alarmDisabled = true;
  Serial.println("Alarm disabled by button press.");
}

// Prekidna rutina za tipku koja omogućuje alarm
void enableAlarmISR() {
  alarmDisabled = false;
  Serial.println("Alarm enabled by button press.");
}

// Prekidna rutina timera
void timerISR() {
  timerTriggered = true;
}
