/**
 * @file complex_smart_lighting_system.ino
 *
 * @mainpage Sustav za pametno osvjetljenje i monitor temperature
 *
 * @section opis Opis
 * Ovaj projekt demonstrira naprednije upravljanje LED osvjetljenjem
 * i praćenje temperature pomoću dva senzora:
 *  - Fotootpornik (LDR, spojen na A0) kontrolira svjetlinu LED diode u AUTO načinu.
 *  - Temperaturni senzor (npr. LM35, spojen na A1) prati temperaturu.
 *
 * Sustav radi u tri načina:
 *   - AUTO način: automatski se podešava svjetlina LED diode.
 *   - RUČNI način: LED dioda na pinu 9 trepće, a način se prebacuje pritiskom tipke na pinu 2.
 *   - ALERT način: ako temperatura prijeđe prag, LED na pinu 13 (crvena upozoravajuća LED)
 *     brzo trepće kao signal upozorenja.
 *
 * Timer1 prekid je konfiguriran tako da se svakih 5 sekundi osvježe očitanja senzora
 * i kontrolira logika prebacivanja načina.
 *
 * @section hardver Hardversko postavljanje
 * - LED (normalna) spojena na pin 9 (PWM izlaz)
 * - Upozoravajuća LED (crvena) spojena na pin 13
 * - Fotootpornik spojen u djeliteljskom krugu na A0
 * - Temperaturni senzor (npr. LM35) spojen na A1
 * - Tipkalo spojeno na pin 2 (INT0) s aktivnim internim pull-up otpornikom
 *
 * @section autor Autor
 * - Kreirano [Branimir Kuveždić] na 09.04.2025.
 */

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>

// Definicija načina rada
enum SystemState { AUTO, MANUAL, ALERT };
volatile SystemState systemState = AUTO;

// Zastavice za prekide
volatile bool modeToggleFlag = false;
volatile bool timerFlag = false;

// Definicije pinova
const int ledPin      = 9;   // Normalna LED (PWM kontrola)
const int alertLEDPin = 13;  // Upozoravajuća LED (ALERT način)
const int buttonPin   = 2;   // Tipkalo za prebacivanje načina
const int ldrPin      = A0;  // Fotootpornik
const int tempPin     = A1;  // Temperaturni senzor

// Varijable za debouncing tipke
unsigned long lastToggleTime = 0;
const unsigned long debounceDelay = 50; // 50 ms debounce

// Brojač ručnih ciklusa; nakon određenog broja, sustav se vraća u AUTO način
int manualCycleCount = 0;
const int manualCycleLimit = 3;

// Temperaturni prag (npr. 30 °C)
float temperatureThreshold = 30.0;

// Timer1 prekidna rutina (svakih 5 sekundi)
ISR(TIMER1_OVF_vect) {
  timerFlag = true;
  TCNT1 = 0;
}

// Prekidna rutina za tipku
void buttonISR() {
  modeToggleFlag = true;
}

// Konfiguracija Timer1 za približno 5 sekundi (ovisno o frekvenciji procesora, 16 MHz)
void setupTimer1() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  
  // Postavi prescaler na 1024
  TCCR1B |= (1 << CS12) | (1 << CS10);
  
  // Omogući overflow prekid
  TIMSK1 |= (1 << TOIE1);
  interrupts();
}

void setup() {
  Serial.begin(9600);
  
  // Konfiguracija pinova
  pinMode(ledPin, OUTPUT);
  pinMode(alertLEDPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  
  // Vanjski prekid za tipku (INT0)
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);
  
  // Inicijalizacija Timer1
  setupTimer1();
  
  Serial.println(F("=== Složeniji sustav za pametno osvjetljenje i monitor temperature ==="));
  Serial.println(F("Pritisnite tipku na pinu 2 za prebacivanje između automatskog i ručnog načina rada."));
  Serial.println(F("Senzori: fotootpornik (A0) i temperaturni senzor (A1)."));
}

void loop() {
  // Obrada prekida tipke s debouncingom
  if (modeToggleFlag && (millis() - lastToggleTime >= debounceDelay)) {
    if(systemState == AUTO) {
      systemState = MANUAL;
      manualCycleCount = 0;
      Serial.println(F("Prebačeno u RUČNI način rada."));
    }
    else if(systemState == MANUAL) {
      systemState = AUTO;
      Serial.println(F("Prebačeno u AUTOMATSKI način rada."));
    }
    // Reset zastavice i postavi vrijeme posljednjeg pritiska
    modeToggleFlag = false;
    lastToggleTime = millis();
  }
  
  // Periodično očitavanje senzora svakih 5 sekundi (Timer1 prekid)
  if(timerFlag) {
    timerFlag = false;
    
    int ldrValue = analogRead(ldrPin);
    int tempValue = analogRead(tempPin);
    
    // Pretvorba očitanja temperaturnog senzora u °C
    // (Za LM35, 10 mV/°C; ADC: 0-1023 za 0-5V)
    float temperature = (tempValue * 500.0) / 1023;
    
    Serial.print(F("Senzor LDR: "));
    Serial.print(ldrValue);
    Serial.print(F(" | Temperatura: "));
    Serial.print(temperature);
    Serial.println(F(" °C"));
    
    // Provjera temperature – ako je previsoka, prebacuje se u ALERT način
    if(temperature >= temperatureThreshold) {
      systemState = ALERT;
      Serial.println(F("ALERT: Previsoka temperatura detektirana!"));
    }
    
    // U ručnom načinu, broji se ciklusi i nakon određenog broja se automatski vraća u AUTO način
    if(systemState == MANUAL) {
      manualCycleCount++;
      if(manualCycleCount >= manualCycleLimit) {
        systemState = AUTO;
        manualCycleCount = 0;
        Serial.println(F("Prekoračen broj ručnih ciklusa, prebacivanje u AUTOMATSKI način rada."));
      }
    }
  }
  
  // Izvršavanje ponašanja ovisno o načinu rada
  if(systemState == AUTO) {
    // AUTO način: LED svjetli proporcionalno očitanju s fotootpornika
    int sensorVal = analogRead(ldrPin);
    int pwmVal = map(sensorVal, 0, 1023, 255, 0); // Tamnija okolina -> jači LED
    analogWrite(ledPin, pwmVal);
    
    // U ALERT načinu se koristi zasebna LED, pa se u AUTO osigurava da upozoravajuća LED gori isključena
    digitalWrite(alertLEDPin, LOW);
    
    Serial.print(F("AUTO način: LED PWM = "));
    Serial.println(pwmVal);
  }
  else if(systemState == MANUAL) {
    // RUČNI način: LED trepće u fiksnom intervalu
    static unsigned long lastBlinkTime = 0;
    static bool ledState = false;
    if(millis() - lastBlinkTime >= 250) {
      ledState = !ledState;
      digitalWrite(ledPin, ledState ? HIGH : LOW);
      lastBlinkTime = millis();
    }
    digitalWrite(alertLEDPin, LOW);
    Serial.println(F("RUČNI način: LED trepće."));
  }
  else if(systemState == ALERT) {
    // ALERT način: Normalna LED se gasi, dok upozoravajuća LED (crvena) trepće brže
    digitalWrite(ledPin, LOW);
    static unsigned long lastAlertTime = 0;
    static bool alertState = false;
    if(millis() - lastAlertTime >= 100) {
      alertState = !alertState;
      digitalWrite(alertLEDPin, alertState ? HIGH : LOW);
      lastAlertTime = millis();
    }
    Serial.println(F("ALERT način: Previsoka temperatura! Trepće crvena LED."));
  }
  
  // Kratko odgađanje kako se ne bi preopteretio serijski monitor
  delay(100);
}
