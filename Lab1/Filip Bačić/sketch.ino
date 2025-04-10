#include <Arduino.h>
#include <Ticker.h>

/**
 * @brief Definicija pinova za tipkala, LED diodu, senzor i serijsku komunikaciju.
 */
#define BTN1_PIN 12
#define BTN2_PIN 14
#define LED_PIN 2
#define SENSOR_PIN 33
#define TRIG_PIN 5
#define ECHO_PIN 18
#define SERIAL_BAUD_RATE 115200
#define SERIAL_RX_PIN 16

/**
 * @brief Varijable za upravljanje stanjima tipkala, senzora, timera i serijske komunikacije.
 */
volatile bool btn1_pressed = false;
volatile bool btn2_pressed = false;
volatile bool sensor_triggered = false;
volatile bool timer_triggered = false;
volatile bool serial_received = false;
char received_char;

unsigned long lastBtn1Press = 0;
unsigned long lastBtn2Press = 0;
const unsigned long debounceDelay = 200;

Ticker timer;
bool ledSensorState = false;

unsigned long lastSensorBlinkTime = 0;
const unsigned long sensorBlinkInterval = 200;

/**
 * @brief ISR za pritisak tipkala 1.
 * 
 * Ova ISR rutina se aktivira kada je tipkalo 1 pritisnuto.
 */
void IRAM_ATTR btn1ISR() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastBtn1Press > debounceDelay) {
        btn1_pressed = true;
        lastBtn1Press = currentMillis;
    }
}

/**
 * @brief ISR za pritisak tipkala 2.
 * 
 * Ova ISR rutina se aktivira kada je tipkalo 2 pritisnuto.
 */
void IRAM_ATTR btn2ISR() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastBtn2Press > debounceDelay) {
        btn2_pressed = true;
        lastBtn2Press = currentMillis;
    }
}

/**
 * @brief ISR za aktivaciju senzora.
 * 
 * Ova ISR rutina se aktivira kada senzor detektira promjenu stanja.
 */
void IRAM_ATTR sensorISR() {
    sensor_triggered = true;
}

/**
 * @brief Funkcija koja se poziva svakih 1 sekundu (TIMER1) za blikanje LED diode.
 * 
 * Ova funkcija se koristi za preklapanje stanja LED diode svakih 1 sekundu.
 */
void IRAM_ATTR onTimer() {
    timer_triggered = true;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Preklopi stanje LED-a
}

/**
 * @brief ISR za serijsku komunikaciju.
 * 
 * Ova ISR rutina se aktivira kada se primi podatak sa serijskog porta.
 */
void IRAM_ATTR serialISR() {
    if (Serial.available()) {
        received_char = Serial.read();
        serial_received = true;
    }
}

/**
 * @brief Funkcija za mjerenje udaljenosti pomoću HC-SR04 senzora.
 * 
 * Funkcija šalje impuls na TRIG pin, mjeri vrijeme trajanja odgovora na ECHO pin,
 * i vraća udaljenost u centimetrima.
 * 
 * @return Udaljenost u centimetrima ili -1 ako mjerenje nije uspjelo.
 */
float measureDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration == 0) return -1;

    float distance = duration * 0.034 / 2;
    return distance;
}

/**
 * @brief Funkcija za treptanje LED diode kada je udaljenost manja od 100 cm.
 * 
 * Ova funkcija uključuje i isključuje LED diodu na temelju mjerenja udaljenosti.
 */
void blinkSensorLED() {
    ledSensorState = !ledSensorState;
    digitalWrite(LED_PIN, ledSensorState);
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    pinMode(BTN1_PIN, INPUT_PULLUP); // Postavljanje pinova za tipkala
    pinMode(BTN2_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT); // Postavljanje pina za LED diodu
    pinMode(SENSOR_PIN, INPUT); // Postavljanje pina za PIR senzor
    pinMode(TRIG_PIN, OUTPUT); // Postavljanje pina za HC-SR04 trig
    pinMode(ECHO_PIN, INPUT); // Postavljanje pina za HC-SR04 echo

    attachInterrupt(BTN1_PIN, btn1ISR, RISING); // Postavljanje prekida za tipkalo 1
    attachInterrupt(BTN2_PIN, btn2ISR, RISING); // Postavljanje prekida za tipkalo 2
    attachInterrupt(SENSOR_PIN, sensorISR, CHANGE); // Postavljanje prekida za PIR senzor
    attachInterrupt(SERIAL_RX_PIN, serialISR, FALLING); // Postavljanje prekida za serijsku komunikaciju

    timer.attach(1.0, onTimer); // Postavljanje timera koji poziva onTimer funkciju svake sekunde
}

void loop() {
    // Obrada događaja kada je tipkalo 1 pritisnuto
    if (btn1_pressed) {
        btn1_pressed = false;
        digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Preklopi stanje LED-a
        Serial.println("Button 1 Pressed: LED toggled");
    }

    // Obrada događaja kada je tipkalo 2 pritisnuto
    if (btn2_pressed) {
        btn2_pressed = false;
        digitalWrite(LED_PIN, HIGH); // Uključi LED
        Serial.println("Button 2 Pressed: LED ON");
    }

    // Obrada događaja kada je PIR senzor aktiviran
    if (sensor_triggered) {
        sensor_triggered = false;
        Serial.println("Motion Detected by PIR Sensor");
    }

    // Obrada događaja kada je timer aktiviran
    if (timer_triggered) {
        timer_triggered = false;
        Serial.println("Timer Interrupt Triggered");
    }

    // Obrada podataka sa serijskog porta
    if (serial_received) {
        serial_received = false;
        Serial.print("Received from Serial: ");
        Serial.println(received_char);
    }

    // Mjerenje udaljenosti pomoću HC-SR04
    float distance = measureDistance();
    if (distance > 0 && distance < 100) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastSensorBlinkTime >= sensorBlinkInterval) {
            blinkSensorLED(); // Trepće LED ako je udaljenost manja od 100 cm
            lastSensorBlinkTime = currentMillis;
        }
    }
}
