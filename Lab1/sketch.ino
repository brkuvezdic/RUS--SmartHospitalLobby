const int pirPin = 2;
const int ldrPin = A0;
const int buzzerPin = 3;
const int ledPin = 4;
const int buttonPin = 5;

volatile bool motionDetected = false;
volatile bool alarmDisabled = false;

void setup() {
  pinMode(pirPin, INPUT);
  pinMode(ldrPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(pirPin), motionISR, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);
  Serial.begin(9600);
}

void loop() {
  int lightLevel = analogRead(ldrPin);
  Serial.print("Light Level: ");
  Serial.println(lightLevel);
  
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
  delay(500);
}

void motionISR() {
  motionDetected = true;
}

void buttonISR() {
  alarmDisabled = true;
  Serial.println("Alarm disabled by button press.");
}
