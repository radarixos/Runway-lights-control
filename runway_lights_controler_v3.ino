// RUNWAY LIGHTS CONTROL
//
// date:12/2024, ver.:3
// non-commercial use only, (cc)OK1VBR
//
// usecase:
// if pilot three times click to PTT (like as morse S)
// runway lights for approach and landing turn ON.
// signal is injected from airman radio.
// table for times:
// LHH - 15sec (D10 to GND, D11 and D12 PULLUP)
// HLH - 1min  (D10 PULLUP, D11 to GND, D12 PULLUP)
// LLH - 5min  (D10 and D11 to GND, D12 PULLUP)
// HHL - 10min (D10 and D11 PULLUP, D12 to GND
// HHH - 15min (default, all jumps OPEN)
// 
// developed for buddies from Kociciny airstripe.
// based on Arduino NANO modul and relay board.


// Definice pinů
const int vstupniPin = 2; // Pin pro vstup pulzů
const int relePin = 4; // Pin pro ovládání relé
const int ledVstupPin = 3; // Externí LED kopíruje stav vstupu
const int externiLedVystupPin = 5; // Další externí LED kopíruje stav výstupu
const int pinCas1 = 10; // Pin pro časovou konfiguraci 1
const int pinCas2 = 11; // Pin pro časovou konfiguraci 2
const int pinCas3 = 12; // Pin pro časovou konfiguraci 3

// Konstanta pro časový limit mezi pulzy a dobu trvání HIGH
const unsigned long minimalniDelkaPulzu = 25; // Minimální délka pulzu v ms
const unsigned long maximalniDelkaPulzu = 1000; // Maximální délka pulzu v ms
const unsigned long minimalniInterval = 10; // Minimální interval mezi pulzy v ms
const unsigned long maximalniInterval = 1000; // Maximální interval mezi pulzy v ms

// Proměnné pro sledování stavu
unsigned long posledniCasPulzu = 0;
unsigned long casZacatkuPulzu = 0;
int pocetPulzu = 0;
bool vystupAktivni = false;
unsigned long casZacatkuHigh = 0;

void setup() {
  pinMode(vstupniPin, INPUT_PULLUP);
  pinMode(relePin, OUTPUT);
  pinMode(ledVstupPin, OUTPUT);
  pinMode(externiLedVystupPin, OUTPUT);
  pinMode(pinCas1, INPUT_PULLUP);
  pinMode(pinCas2, INPUT_PULLUP);
  pinMode(pinCas3, INPUT_PULLUP);
  digitalWrite(relePin, HIGH); // Obrácená logika relé
  digitalWrite(ledVstupPin, LOW);
  digitalWrite(externiLedVystupPin, LOW);
}

unsigned long nastavDobaHigh() {
  int stav1 = digitalRead(pinCas1);
  int stav2 = digitalRead(pinCas2);
  int stav3 = digitalRead(pinCas3);

  //jumpery LOW=short, HIGH=open
  if (stav1 == LOW && stav2 == HIGH && stav3 == HIGH) return 15000; // 15 sekund
  if (stav1 == HIGH && stav2 == LOW && stav3 == HIGH) return 60000; // 1 minuta
  if (stav1 == LOW && stav2 == LOW && stav3 == HIGH) return 300000; // 5 minut
  if (stav1 == HIGH && stav2 == HIGH && stav3 == LOW) return 600000; // 10 minut
  // Přidej další kombinace podle potřeby
  return 9000000; // Výchozí hodnota 15 minut, vsechny jumpery OPEN.
}

void loop() {
  unsigned long aktualniCas = millis();

  // Zkontroluj, zda přišel pulz
  if (digitalRead(vstupniPin) == HIGH) {
    // Začátek pulzu
    if (casZacatkuPulzu == 0) {
      casZacatkuPulzu = aktualniCas;
    }

    // Pokud je délka pulzu příliš dlouhá, ignoruj
    if (aktualniCas - casZacatkuPulzu > maximalniDelkaPulzu) {
      casZacatkuPulzu = 0;
    }

    // Zhasnutí LED pro vstup (obrácená logika)
    digitalWrite(ledVstupPin, LOW);
  } else if (casZacatkuPulzu > 0) {
    // Konec pulzu
    unsigned long delkaPulzu = aktualniCas - casZacatkuPulzu;
    casZacatkuPulzu = 0;

    // Rozsvícení LED pro vstup (obrácená logika)
    digitalWrite(ledVstupPin, HIGH);

    // Otestuj, zda je pulz v platném rozmezí
    if (delkaPulzu >= minimalniDelkaPulzu && delkaPulzu <= maximalniDelkaPulzu) {
      // Zkontroluj interval mezi pulzy
      unsigned long interval = aktualniCas - posledniCasPulzu;

      if (pocetPulzu == 0 || (interval >= minimalniInterval && interval <= maximalniInterval)) {
        pocetPulzu++;
        posledniCasPulzu = aktualniCas;

        // Pokud přišly tři pulzy, aktivuj nebo deaktivuj výstup
        if (pocetPulzu == 3) {
          if (!vystupAktivni) {
            digitalWrite(externiLedVystupPin, HIGH);
            digitalWrite(relePin, LOW); // Obrácená logika relé
            vystupAktivni = true;
            casZacatkuHigh = aktualniCas;
          } else {
            digitalWrite(externiLedVystupPin, LOW);
            digitalWrite(relePin, HIGH); // Obrácená logika relé
            vystupAktivni = false;
          }
          pocetPulzu = 0; // Resetuj počítadlo pulzů
        }
      } else {
        // Resetuj počítadlo, pokud interval nevyhovuje
        pocetPulzu = 1;
        posledniCasPulzu = aktualniCas;
      }
    }
  }

  // Pokud je výstup aktivní, zkontroluj, zda uplynula nastavená doba
  if (vystupAktivni && (millis() - casZacatkuHigh >= nastavDobaHigh())) {
    digitalWrite(externiLedVystupPin, LOW);
    digitalWrite(relePin, HIGH); // Obrácená logika relé
    vystupAktivni = false;
  }
}
