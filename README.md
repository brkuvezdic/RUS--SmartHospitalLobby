# RUS--SmartHospitalLobby

## Pregled projekta
SmartHospitalLobby je sustav za prijavu pacijenata dizajniran za pojednostavljenje procesa dolaska u bolnicu. Kada pacijenti dođu, mogu ručno unijeti svoj identifikacijski broj zdravstvne iskaznice, nakon čega sustav provjerava njihov termin za taj dan i pruža relevantne informacije o njihovom posjetu (kat, soba, vrijeme). Dodatno, bolnički sustav prima potvrdu o dolasku pacijenta, što pomaže smanjiti gužve i poboljšati organizaciju prijema pacijenata.

## Implementirane funkcionalnosti
- **Identifikacija pacijenta**: Ručni unos identifikacijskih brojeva pacijenata putem tipkovnice
- **Provjera termina**: Sustav provjerava ima li pacijent zakazan termin za taj dan
- **Prikaz informacija**: Prikazuje broj sobe, vrijeme termina i ime liječnika na LCD zaslonu
- **Podrška za dvije baze podataka**: 
  - Online način rada: Povezivanje s Firebase bazom podataka u stvarnom vremenu 
  - Offline način rada: Lokalna sigurnosna kopija baze podataka kada se izgubi internetska veza
- **Indikatori statusa**: LED indikatori za uspješnu i neuspješnu identifikaciju
- **Upravljanje neaktivnošću**: Sustav se vraća u početno stanje nakon perioda neaktivnosti
- **Wi-Fi povezivost**: Automatsko povezivanje s pokušajima ponovnog povezivanja i prebacivanjem na offline način rada
- **Korisničko sučelje**: 16x2 LCD zaslon s intuitivnim navigacijskim sustavom
- **Rukovanje greškama**: Jasne poruke o greškama i indikatori statusa sustava

## Hardverske komponente
- ESP32 razvojna ploča
- 16x2 I2C LCD zaslon
- 4x4 membranska tipkovnica
- Status LED (zelena za uspjeh, crvena za grešku)
- Wi-Fi modul (integriran u ESP32)

## Korištene tehnologije
- **Mikrokontroler**: ESP32 (za obradu, Wi-Fi povezivost)
- **Zaslon**: LCD putem I2C sučelja
- **Unos**: Membranska tipkovnica (4x4)
- **Baza podataka**: Firebase Realtime Database
- **Komunikacija**: WiFi za online povezivost
- **Razvojno okruženje**: Arduino IDE / Wokwi za simulaciju
- **Biblioteke**:
  - Wire (I2C komunikacija)
  - LiquidCrystal_I2C (kontrola LCD-a)
  - WiFi (mrežna povezivost)
  - FirebaseESP32 (Firebase integracija)
  - ArduinoJson (JSON parsiranje)

## Arhitektura sustava
Sustav radi u sljedećim stanjima:
1. **WAITING_FOR_INPUT**: Sustav čeka da pacijent unese broj svoje kartice
2. **PROCESSING_CARD**: Provjerava informacije o pacijentu u bazi podataka
3. **DISPLAY_PATIENT_INFO**: Prikazuje relevantne informacije o terminu
4. **DISPLAY_ERROR**: Prikazuje poruku o grešci kada informacije o pacijentu nisu pronađene

## Mogućnost rada bez interneta
Kada internetska veza nije dostupna, sustav se automatski prebacuje na offline način rada, koristeći lokalnu bazu podataka o pacijentima pohranjenu u memoriji uređaja.

## Instalacija i postavljanje
1. Klonirajte ovaj repozitorij
2. Otvorite projekt u Arduino IDE ili Wokwi simulatoru
3. Konfigurirajte svoje WiFi podatke i Firebase postavke u kodu
4. Prenesite kod na svoj ESP32 uređaj
5. Spojite hardverske komponente prema konfiguraciji pinova u kodu

## Konfiguracija pinova
- **LCD I2C**: SDA -> ESP32 pin 21, SCL -> ESP32 pin 22
- **Tipkovnica**: 
  - Redovi: 12, 13, 14, 15
  - Stupci: 5, 18, 19, 23
- **LED**:
  - LED uspjeha: ESP32 pin 2
  - LED greške: ESP32 pin 4

## Struktura baze podataka
Firebase baza podataka treba imati sljedeću strukturu:
```
/pacijenti/
    /{broj_kartice_pacijenta}/
        fullName: "Puno ime pacijenta"
        roomNumber: "Broj sobe"
        appointmentTime: "HH:MM"
        doctor: "Ime liječnika"
```

## Upute za korištenje
1. Sustav prikazuje "Unesite broj:" pri pokretanju
2. Pacijent unosi broj svoje zdravstvene iskaznice koristeći tipkovnicu
3. Pritisnite '#' za potvrdu unosa, '*' za brisanje posljednjeg znaka
4. Sustav provjerava podatke o pacijentu i prikazuje detalje termina ako su pronađeni
5. Pritisnite 'D' za povratak na zaslon za unos
6. Sustav se automatski vraća u način unosa nakon perioda neaktivnosti

## Buduća poboljšanja
- Integracija RFID čitača kartica
- Podrška za više jezika
- Sustav obavještavanja liječnika
- Prikaz pozicije u redu
- SMS obavijesti pacijentima


## Suradnici
Branimir Kuveždić
Filip Bačić
Martin Hrupec
