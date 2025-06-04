#include "csma_cd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
  Symulator CSMA/CD z jammowaniem, w którym:
    - Kolizję (#) wyświetlamy tylko w tym takcie, w którym faktycznie się zdarzyła.
      ---> Komórki o wartości -1 (kolizja) NIE PROPOGUJĄ się dalej!
    - Jam (@) jest sygnałem aktywnie nadawanym przez stację i propaguje się,
      aż osiągnie cały kanał, żeby „wyprzeć” fale kolizji.
    - Każda stacja wysyła dane (litera A/B/...) przez dokładnie frame_len taktów,
      wykrywa kolizję natychmiast w swojej pozycji i przechodzi w jamowanie
      na 2 * channel_len taktów, a potem ponawia ramkę od nowa.
*/

void run_simulation(int channel_len,
                    int num_hosts,
                    const char host_name[],
                    const int host_pos[],
                    const int host_start[],
                    int frame_len,
                    int max_ticks)
{
    const int JAM_INDEX = num_hosts;

    /* 
      W każdej komórce kanału trzymamy „kod pochodzenia” sygnału:
        -2           → komórka pusta (drukujemy '.')
        -1           → kolizja                (drukujemy '#')
        0..num_hosts-1 → sygnał od danej stacji (drukujemy host_name[idx])
        JAM_INDEX    → sygnał jamujący        (drukujemy '@')
    */
    int *curr_origin = malloc(channel_len * sizeof(int));
    int *next_origin = malloc(channel_len * sizeof(int));
    if (!curr_origin || !next_origin) {
        fprintf(stderr, "Błąd: nie udało się zaalokować pamięci na tablice origin.\n");
        free(curr_origin);
        free(next_origin);
        return;
    }

    /*
      Dla każdej stacji h trzymamy:
        remaining_data[h] – ile taktów pozostało do wysłania danych (0 = nie nadaje teraz)
        jam_ticks[h]      – ile taktów jeszcze trwa jammowanie (0 = nie jammuje)
    */
    int *remaining_data = calloc(num_hosts, sizeof(int));
    int *jam_ticks      = calloc(num_hosts, sizeof(int));
    if (!remaining_data || !jam_ticks) {
        fprintf(stderr, "Błąd: nie udało się zaalokować wewnętrznych tablic stacji.\n");
        free(curr_origin);
        free(next_origin);
        free(remaining_data);
        free(jam_ticks);
        return;
    }

    /* Na starcie wszystkie komórki puste: */
    for (int i = 0; i < channel_len; i++) {
        curr_origin[i] = -2;
    }

    /* Tablica informująca, która komórka jest pozycją stacji: */
    char *is_host_pos = calloc(channel_len, sizeof(char));
    if (!is_host_pos) {
        fprintf(stderr, "Błąd: nie udało się zaalokować is_host_pos.\n");
        free(curr_origin);
        free(next_origin);
        free(remaining_data);
        free(jam_ticks);
        return;
    }
    for (int h = 0; h < num_hosts; h++) {
        int p = host_pos[h];
        if (p >= 0 && p < channel_len) {
            is_host_pos[p] = 1;
        }
    }

    /* Główna pętla symulacji: takty 0..max_ticks-1 */
    for (int tick = 0; tick < max_ticks; tick++) {
        /* 1) W takt startu danej stacji, jeśli nie nadaje i nie jammuje, uruchamiamy jej frame */
        for (int h = 0; h < num_hosts; h++) {
            if (tick == host_start[h] && remaining_data[h] == 0 && jam_ticks[h] == 0) {
                remaining_data[h] = frame_len;
            }
        }

        /* 2) Przygotujmy tablice pomocnicze „sending[]” oraz „sending_type[]” */
        char *sending      = calloc(channel_len, sizeof(char));
        int  *sending_type = malloc(channel_len * sizeof(int));
        if (!sending || !sending_type) {
            fprintf(stderr, "Błąd: nie udało się zaalokować tablic pomocniczych.\n");
            free(curr_origin);
            free(next_origin);
            free(remaining_data);
            free(jam_ticks);
            free(is_host_pos);
            free(sending);
            free(sending_type);
            return;
        }
        for (int i = 0; i < channel_len; i++) {
            sending_type[i] = -2;  /* -2 → nikt nie nadaje */
        }

        /* Ustalmy, które stacje nadają teraz:
           - Jeśli jam_ticks[h] > 0 → stacja h właśnie nadaje jam (@)
           - Else jeśli remaining_data[h] > 0 → stacja h nadaje dane (host_name[h])
        */
        for (int h = 0; h < num_hosts; h++) {
            int p = host_pos[h];
            if (p < 0 || p >= channel_len) continue;
            if (jam_ticks[h] > 0) {
                sending[p] = 1;
                sending_type[p] = JAM_INDEX;
            } else if (remaining_data[h] > 0) {
                sending[p] = 1;
                sending_type[p] = h;
            }
        }

        /* 3) Wyzeruj next_origin (wszystkie komórki = -2) */
        for (int i = 0; i < channel_len; i++) {
            next_origin[i] = -2;
        }

        /* 4) Propagacja fali z curr_origin → next_origin
             - Propagujemy TYLKO, jeśli origin >= 0 (dane) lub origin == JAM_INDEX (jam).
             - Jeśli origin == -1 (kolizja), to nie propagujemy tego dalej.
             - Jeśli docelowa komórka jest aktualnie zajęta nadawaniem (sending[...] == 1),
               albo jest pozycją stacji, która w tej chwili jest idle (remaining_data == 0 && jam_ticks == 0),
               to blokujemy propagację do tej komórki.
             - Gdy do tej samej komórki próbuje wejść więcej niż jedno „origin”,
               ustawiamy next_origin[...] = -1 (kolizja).
        */
        /* Przyda się tablica host_of_cell[], żeby od razu wiedzieć, czy to pozycja jakiejś stacji. */
        int *host_of_cell = malloc(channel_len * sizeof(int));
        if (!host_of_cell) {
            fprintf(stderr, "Błąd: nie udało się zaalokować host_of_cell.\n");
            free(curr_origin);
            free(next_origin);
            free(remaining_data);
            free(jam_ticks);
            free(is_host_pos);
            free(sending);
            free(sending_type);
            return;
        }
        for (int i = 0; i < channel_len; i++) {
            host_of_cell[i] = -1;
        }
        for (int h = 0; h < num_hosts; h++) {
            int p = host_pos[h];
            if (p >= 0 && p < channel_len) {
                host_of_cell[p] = h;
            }
        }

        for (int i = 0; i < channel_len; i++) {
            int origin = curr_origin[i];
            /* Pomijaj kolizje (#) i puste komórki: */
            if (origin < 0 && origin != JAM_INDEX) continue;

            int left  = i - 1;
            int right = i + 1;

            /* Propagacja w lewo */
            if (left >= 0) {
                int owner = host_of_cell[left];
                /* Blokujemy, jeśli w tej chwili nadawanie w `left` (sending[left]==1) lub
                   jeśli jest to pozycja stacji (owner != -1) i ta stacja jest idle */
                if (!sending[left]
                    && !(owner != -1 && remaining_data[owner] == 0 && jam_ticks[owner] == 0))
                {
                    if (next_origin[left] == -2) {
                        next_origin[left] = origin;
                    } else if (next_origin[left] != origin) {
                        next_origin[left] = -1;
                    }
                }
            }
            /* Propagacja w prawo */
            if (right < channel_len) {
                int owner = host_of_cell[right];
                if (!sending[right]
                    && !(owner != -1 && remaining_data[owner] == 0 && jam_ticks[owner] == 0))
                {
                    if (next_origin[right] == -2) {
                        next_origin[right] = origin;
                    } else if (next_origin[right] != origin) {
                        next_origin[right] = -1;
                    }
                }
            }
        }
        free(host_of_cell);

        /* 5) Dodajemy „świeże” nadania danych (litera) lub jam (@) do next_origin[] */
        for (int i = 0; i < channel_len; i++) {
            if (!sending[i]) continue;
            int origin = sending_type[i];  // 0..num_hosts-1 → dane, JAM_INDEX → jam
            if (next_origin[i] == -2) {
                next_origin[i] = origin;
            } else if (next_origin[i] != origin) {
                next_origin[i] = -1;
            }
        }

        /* 6) Wypisujemy stan kanału (pozycje 0..channel_len-1) */
        /* -2 → '.',  -1 → '#',  JAM_INDEX → '@',  0..num_hosts-1 → host_name[origin] */
        printf("Takt %3d:  ", tick);
        for (int i = 0; i < channel_len; i++) {
            int o = next_origin[i];
            if (o == -2) {
                putchar('.');
            } else if (o == -1) {
                putchar('#');
            } else if (o == JAM_INDEX) {
                putchar('@');
            } else {
                putchar(host_name[o]);
            }
        }
        putchar('\n');

        /* 7) Zaktualizujmy stany poszczególnych stacji:
            - Jeśli sending_type[p] == h (stacja h w tym takcie nadawała dane):
                * Jeżeli next_origin[p] != h  → kolizja w jej komórce:
                    remaining_data[h] = 0;
                    jam_ticks[h] = 2 * channel_len;
                * Else (next_origin[p] == h) → poprawne nadanie 1 taktu danych:
                    remaining_data[h]--;
            - Else jeśli sending_type[p] == JAM_INDEX (stacja h właśnie jammuje):
                jam_ticks[h]--;
                if (jam_ticks[h] == 0) remaining_data[h] = frame_len;
            - W pozostałych przypadkach stacja idle lub czeka na start → nic się nie zmienia.
        */
        for (int h = 0; h < num_hosts; h++) {
            int p = host_pos[h];
            if (p < 0 || p >= channel_len) continue;

            if (sending_type[p] == h) {
                /* Stacja h nadaje dane */
                if (next_origin[p] != h) {
                    /* Kolizja: przechodzimy od razu w jammowanie */
                    remaining_data[h] = 0;
                    jam_ticks[h] = 2 * channel_len;
                } else {
                    /* Jeden takt danych poprawnie wysłany */
                    remaining_data[h]--;
                }
            }
            else if (sending_type[p] == JAM_INDEX) {
                /* Stacja h właśnie jammuje */
                jam_ticks[h]--;
                if (jam_ticks[h] == 0) {
                    /* Po zakończeniu jammowania stacja ponawia wysłanie całego frame */
                    remaining_data[h] = frame_len;
                }
            }
            /* Idle lub czekanie na host_start → nic nie zmieniamy */
        }

        /* 8) Przygotowanie do następnego taktu: kopiujemy next_origin → curr_origin */
        memcpy(curr_origin, next_origin, channel_len * sizeof(int));

        free(sending);
        free(sending_type);
    }

    free(curr_origin);
    free(next_origin);
    free(remaining_data);
    free(jam_ticks);
    free(is_host_pos);
}
