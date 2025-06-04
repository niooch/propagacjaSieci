#include <stdio.h>
#include "csma_cd.h"

/*
  W tym pliku mamy „zahardcodowane” wartości:
  - długość kanału: 100
  - dwie stacje: A i B
  - pozycje: A → 2, B → 15
  - czasy startu: A → 0, B → 2
  - długość ramki: 6 taktów
  - maksymalna liczba taktów: 15
*/

#define CHANNEL_LEN 100
#define NUM_HOSTS     2
#define FRAME_LEN     6
#define MAX_TICKS    150

/* Tablica z nazwami stacji: 'A', 'B' */
static const char host_name[NUM_HOSTS]  = { 'A',  'B' };

/* Pozycje stacji: A w komórce 2, B w komórce 15 */
static const int host_pos[NUM_HOSTS]    = {  2,    74 };

/* Czasy startu: A zaczyna w takcie 0, B zaczyna w takcie 2 */
static const int host_start[NUM_HOSTS]  = {  0,     2 };

int main(void) {
    /* Wypisanie legendy – jakie mamy stacje, ich pozycje i czasy startu */
    printf("=== CSMA/CD Symulacja z Jammingiem ===\n");
    printf("Dlugosc kanalu: %d komorek\n", CHANNEL_LEN);
    printf("Liczba stacji:  %d\n", NUM_HOSTS);
    printf("Dlugosc ramki (taktow): %d\n", FRAME_LEN);
    printf("Maksymalna liczba taktow: %d\n\n", MAX_TICKS);

    printf("Legenda stacji:\n");
    for (int i = 0; i < NUM_HOSTS; i++) {
        printf("  Stacja %c -> pozycja = %d, start = takt %d\n",
               host_name[i],
               host_pos[i],
               host_start[i]);
    }
    printf("\nRozpoczynam symulacje:\n\n");

    /* Wywołanie symulacji CSMA/CD z jammingiem */
    run_simulation(
        CHANNEL_LEN,
        NUM_HOSTS,
        host_name,
        host_pos,
        host_start,
        FRAME_LEN,
        MAX_TICKS
    );

    return 0;
}
