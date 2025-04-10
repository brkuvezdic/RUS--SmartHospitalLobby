/**
 * @file power_management_system.ino
 * @brief Arduino sustav za upravljanje potrošnjom energije.
 *
 * @mainpage Sustav za upravljanje potrošnjom energije
 *
 * @section opis Opis
 * Ovaj Arduino projekt demonstrira tehnike štednje energije korištenjem različitih sleep
 * modova. Cijan LED bljeska 3 sekunde, nakon čega sustav ulazi u niskoenergetski
 * sleep mode. Buđenje je moguće pomoću tipkala (vanjski prekid) ili nakon
 * vremenskog intervala korištenjem Timer1 (~8 sekundi).
 *
 * @section hardver Hardversko postavljanje
 * - LED povezan na pin 13
 * - Tipkalo povezano na pin 2 (koristi se s vanjskim prekidom INT0)
 *
 * @section biblioteke Biblioteke
 * - AVR Sleep biblioteka
 * - AVR Interrupt biblioteka
 * - AVR Power biblioteka
 *
 * @section napomene Napomene
 * - Tipkalo koristi unutarnji pull-up otpornik i aktivira se na padajućem bridu
 * - Timer1 je konfiguriran za buđenje uređaja približno svakih 8 sekundi
 *
 * @section autor Autor
 * - Kreirano Filip Bačić na 09.04.2025.
 */

// Biblioteke
#include <avr/sleep.h>     ///< Biblioteka za sleep modove
#include <avr/interrupt.h> ///< Biblioteka za rukovanje prekidima
#include <avr/power.h>     ///< Biblioteka za upravljanje energijom

// Definicije pinova
const int ledPin = 13;     ///< Pin spojen na LED
const int buttonPin = 2;   ///< Pin spojen na tipkalo (INT0)

// Globalne varijable
volatile bool wakeFlag = false;  ///< Zastavica koja određuje stanje buđenja
volatile uint8_t sleepMode = 1;  ///< Način sleep moda (1: IDLE, 2: POWER_DOWN, 3: POWER_SAVE)
unsigned long sleepCycles = 0;   ///< Brojač ciklusa spavanja

/**
 * @brief Vanjski prekidni servis za tipkalo
 * Ovaj ISR se aktivira kada je tipkalo pritisnuto (padajući brid),
 * i postavlja zastavicu za buđenje uređaja.
 */
void buttonInterrupt() {
  wakeFlag = true;
}

/**
 * @brief Timer1 prekidni servis rutina
 * Ovaj ISR se automatski aktivira nakon isteka timera,
 * i postavlja zastavicu za buđenje uređaja.
 */
ISR(TIMER1_OVF_vect) {
  wakeFlag = true;
}

/**
 * @brief Arduino setup funkcija
 * Inicijalizira serijsku komunikaciju, postavlja pinove, konfigurira vanjski prekid,
 * i postavlja mikrokontroler za korištenje sleep modova.
 */
void setup() {
  Serial.begin(9600);  ///< Inicijalizacija serijske komunikacije
  
  pinMode(ledPin, OUTPUT);        ///< Postavi LED pin kao izlaz
  pinMode(buttonPin, INPUT_PULLUP); ///< Postavi tipkalo kao ulaz s pull-up otpornikom
  
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonInterrupt, FALLING); ///< Konfiguriraj vanjski prekid na padajući brid
  
  // Prikaži početne informacije
  Serial.println(F("=== Sustav za upravljanje potrošnjom energije ==="));
  Serial.println(F("Tipkalo na pinu 2 za buđenje"));
  Serial.println(F("LED na pinu 13 pokazuje aktivno stanje"));
  Serial.println(F("Sustav će automatski ući u sleep mode nakon 3 sekunde"));
}

/**
 * @brief Arduino glavna loop funkcija
 * Bljeska LED 3 sekunde, zatim ulazi u sleep mode.
 * Budi se na pritisak tipkala ili Timer1 prekid.
 */
void loop() {
  // Aktivno stanje - LED bljeska
  Serial.println(F("\nAktivno stanje - LED bljeska"));
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
  }
  
  // Promjena sleep moda nakon svakih 3 ciklusa
  if (sleepCycles % 3 == 0 && sleepCycles > 0) {
    sleepMode = (sleepMode % 3) + 1;
    Serial.print(F("Promjena sleep moda na: "));
    switch (sleepMode) {
      case 1: Serial.println(F("IDLE")); break;
      case 2: Serial.println(F("POWER_DOWN")); break;
      case 3: Serial.println(F("POWER_SAVE")); break;
    }
  }

  // Ulazak u sleep mode
  Serial.print(F("Ulazim u sleep mode... (#"));
  Serial.print(sleepCycles + 1);
  Serial.println(F(")"));
  delay(100); // Daj vremena serijskoj komunikaciji da završi
  
  goToSleep(); // Ulazi u sleep mode
  
  // Nakon buđenja
  sleepCycles++;
  Serial.println(F("Buđenje!"));
  
  // Prikaži način buđenja
  if (digitalRead(buttonPin) == LOW) {
    Serial.println(F("Buđenje uzrokovano tipkalom"));
  } else {
    Serial.println(F("Buđenje uzrokovano Timerom"));
  }
  
  delay(1000); // Kratak odmor prije ponovnog ciklusa
}

/**
 * @brief Konfigurira Timer1 za buđenje nakon ~8 sekundi
 */
void setupTimer1() {
  // Resetiraj Timer1
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  
  // Postavi prescaler na 1024
  TCCR1B |= (1 << CS12) | (1 << CS10);
  
  // Omogući Timer1 overflow prekid
  TIMSK1 |= (1 << TOIE1);
}

/**
 * @brief Postavlja mikrokontroler u sleep mode do pojave prekida
 */
void goToSleep() {
  // Priprema za sleep mode
  wakeFlag = false;
  
  // Postavi odgovarajući sleep mode
  switch (sleepMode) {
    case 1:
      set_sleep_mode(SLEEP_MODE_IDLE);
      Serial.println(F("IDLE mode - najmanja ušteda energije"));
      break;
    case 2:
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      Serial.println(F("POWER_DOWN mode - najveća ušteda energije"));
      break;
    case 3:
      set_sleep_mode(SLEEP_MODE_PWR_SAVE);
      Serial.println(F("POWER_SAVE mode - srednja ušteda energije"));
      break;
  }
  
  // Postavi Timer1 ako je IDLE ili POWER_SAVE mode
  if (sleepMode != 2) {
    setupTimer1();
  }
  
  // Isključi nepotrebne module za dodatnu uštedu energije
  if (sleepMode > 1) {
    power_adc_disable();
    power_spi_disable();
    power_twi_disable();
  }
  
  // Ugasi LED prije spavanja
  digitalWrite(ledPin, LOW);
  
  noInterrupts();      // Onemogući prekide tijekom pripreme
  sleep_enable();      // Omogući sleep mode
  interrupts();        // Ponovno omogući prekide prije spavanja
  
  sleep_cpu();         // Stavi CPU u sleep mode
  
  // Nastavak izvršavanja nakon buđenja
  sleep_disable();     // Onemogući sleep nakon buđenja
  
  // Ponovno omogući module nakon buđenja
  if (sleepMode > 1) {
    power_all_enable();
  }
}
