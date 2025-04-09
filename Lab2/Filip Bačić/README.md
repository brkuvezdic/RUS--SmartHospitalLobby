# Sustav za upravljanje potrošnjom energije

Ovaj Arduino projekt prikazuje upravljanje potrošnjom energije pomoću različitih sleep modova. Uređaj koristi LED kao indikator aktivnog stanja, a buđenje iz sleep moda ostvaruje se pomoću tipkala (vanjski prekid) ili Timer1 (~8 sekundi).

## 📌 Sadržaj koda

- Inicijalizacija serijske komunikacije (`Serial.begin`)
- Konfiguracija LED na pinu 13 i tipkala na pinu 2 s `INPUT_PULLUP`
- Aktivno stanje: LED bljeska 3 puta (3 sekunde)
- Nakon aktivnog stanja, sustav ulazi u jedan od 3 sleep moda:
  - `SLEEP_MODE_IDLE`
  - `SLEEP_MODE_PWR_DOWN`
  - `SLEEP_MODE_PWR_SAVE`
- Sleep mod se mijenja svakih 3 ciklusa
- ISR za vanjski prekid (`buttonInterrupt`) postavlja `wakeFlag`
- ISR za Timer1 overflow (`TIMER1_OVF_vect`) postavlja `wakeFlag`
- Funkcija `goToSleep()`:
  - Postavlja odgovarajući sleep mode
  - Konfigurira Timer1 (osim za `POWER_DOWN`)
  - Isključuje nepotrebne periferije (`ADC`, `SPI`, `TWI`) u modovima 2 i 3
  - Ulazi u sleep (`sleep_cpu`) i ponovno aktivira sve nakon buđenja
- Prikaz poruka u Serial Monitoru o sleep modu, načinu buđenja i ciklusima

## 🧩 Ugrađene biblioteke

- `<avr/sleep.h>`
- `<avr/interrupt.h>`
- `<avr/power.h>`

## 🔌 Shema spoja

- LED (npr. narančasta) → Pin 13 (s otpornikom) i GND
- Tipkalo → Pin 2 i GND (koristi `INPUT_PULLUP`)

## 💻 Testirano u

- [Wokwi Simulatoru](https://wokwi.com/)
- Arduino UNO

## 📁 Sadržaj repozitorija

- `power_management_system.ino`
- `diagram.json` (Wokwi shema)
- `README.md`

## 👤 Autor

Filip – 09.04.2025.

## 📜 Licenca

MIT License
