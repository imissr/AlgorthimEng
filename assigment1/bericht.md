# Bericht zur Übung: Computer Systems und OpenMP

**Name:**  
**Matrikelnummer:**  
**Datum:**

---

## 1. Ausgewählte Vorlesungsfolie und Erläuterung

### 1.1 Gewählte Folie

Ich beziehe mich auf die Folie **„Why Parallel Computing? – The Free Lunch Is Over“** aus der Vorlesung.

### 1.2 Inhalt der Folie

- Früher wurden Prozessoren stetig schneller, vor allem durch höhere **Taktfrequenzen** und bessere **Mikroarchitekturen**.
- Programmiererinnen und Programmierer mussten ihren Code nicht ändern:  
  Sequentielle Programme wurden auf neuer Hardware automatisch schneller.  
  → Das ist der sogenannte **free lunch**.
- Aufgrund physikalischer Grenzen (Stromverbrauch, Wärmeentwicklung) können Taktfrequenzen heute nicht mehr
  beliebig erhöht werden.
- Stattdessen werden immer mehr **Kerne pro Chip** eingesetzt und **SIMD-/Vektoreinheiten** ausgebaut.

### 1.3 Warum das wichtig ist

Die Aussage **„The free lunch is over“** bedeutet:

- Sequentielle Programme werden auf modernen Prozessoren **nicht mehr automatisch** schneller.
- Wer mehr Leistung haben will, muss **Parallelität explizit nutzen**:

    - mehrere Threads / Prozesse,
    - OpenMP, Tasks, etc.,
    - vektorisierte Berechnungen,
    - cachefreundliche Speicherzugriffe.


---

## 2. Zwei interessante Punkte aus Kapitel 1 von „Computer Systems: A Programmer’s Perspective“

### 2.1 „Information ist Bits + Kontext“

Im Kapitel wird betont, dass ein Computer letztlich nur mit **Bitmustern** arbeitet:

- Die Hardware sieht nur Folgen von 0 und 1.
- Ob ein bestimmtes Bitmuster
    - eine ganze Zahl,
    - eine Gleitkommazahl,
    - ein Zeichen,
    - eine Maschineninstruktion
      darstellt, hängt nur vom **Kontext** ab, in dem wir es interpretieren.

**Warum ich das interessant finde:**

- Es erklärt viele typische Fehler:
    - Ganzzahlüberlauf (overflow),
    - merkwürdige Werte beim Lesen von Speicher mit falschem Typ,
    - Probleme bei Casts und Zeigern.
- Der Rechner „weiß“ nicht, was sinnvoll oder „falsch“ ist. er wendet nur Regeln auf Bits an.
- Dieses Verständnis ist wichtig, um später mit
    - Datenrepräsentationen,
    - Zeigern,
    - Maschinencode
      sinnvoll zu arbeiten.

**Kurz formuliert:**

> Ich fand die Aussage „Information ist Bits + Kontext“ besonders interessant. Die Hardware kennt nur Bitmuster; ob diese ein Integer, ein Float oder ein Zeichen sind, entscheidet allein die Interpretation durch das Programm. Dadurch versteht man, warum manche Fehler keine sofortige Fehlermeldung erzeugen, sondern einfach unsinnige Werte liefern.

---



### 2.2 „Caches Matter“

- Programme verbringen viel Zeit damit, **Daten zu kopieren** (Platte → RAM → CPU).
- **Groß = langsam**, **klein = schnell & teuer** → Prozessor–Speicher-Lücke.
- **Caches (L1, L2, L3)**: kleine, schnelle Zwischenspeicher zwischen CPU und RAM.
- Nutzen **Lokalität**: oft gebrauchte / benachbarte Daten bleiben im Cache → schnellere Zugriffe.
- Wichtig für mich: Performance hängt stark von **Datenlage & Zugriffsmuster** ab. nicht nur von der Anzahl der Operationen.

---

## 3. Parallelisierung des Monte-Carlo-π-Programms (OpenMP)




### 3.3 Beispielimplementierung (C++ mit OpenMP)

```cpp
#include <iostream>
#include <iomanip>
#include <random>
#include <omp.h>

using namespace std;

int main() {
    int n = 100000000;          // Anzahl der Zufallspunkte
    int counter = 0;            // Punkte im Viertelkreis

    double start_time = omp_get_wtime();

    // Parallelbereich
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        // Jeder Thread bekommt seinen eigenen Zufallszahlengenerator
        unsigned int seed = 1234u + tid;
        default_random_engine re(seed);
        uniform_real_distribution<double> zero_to_one(0.0, 1.0);

        int local_counter = 0;  // lokaler Zähler pro Thread

        // Schleife wird auf Threads aufgeteilt
        #pragma omp for
        for (int i = 0; i < n; ++i) {
            double x = zero_to_one(re);
            double y = zero_to_one(re);

            if (x * x + y * y <= 1.0) {
                ++local_counter;
            }
        }

        // Sichere Addition zum globalen Zähler
        #pragma omp atomic
        counter += local_counter;
    }

    double run_time = omp_get_wtime() - start_time;
    double pi = 4.0 * static_cast<double>(counter) / static_cast<double>(n);

    cout << "pi: "       << setprecision(17) << pi       << "\n";
    cout << "run_time: " << setprecision(6)  << run_time << " s\n";
    cout << "n: "        << n << "\n";
}