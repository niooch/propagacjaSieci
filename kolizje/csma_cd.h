#ifndef CSMA_CD_H
#define CSMA_CD_H

#include <stddef.h>  /* dla typu size_t */

/**
 * Uruchamia symulację CSMA/CD na jednokanałowym łączu.
 *
 * @param channel_len  długość kanału (liczba „komórek” w tablicy)
 * @param num_hosts    liczba hostów podłączonych do tego kanału
 * @param host_name    tablica długości num_hosts z literami (np. 'A', 'B', 'C', ...)
 * @param host_pos     tablica długości num_hosts z pozycjami (0..channel_len-1)
 * @param host_start   tablica długości num_hosts z czasami startów (taktów)
 * @param frame_len    długość ramki wysyłanej przez każdy host (w taktach)
 * @param max_ticks    maksymalna liczba taktów symulacji
 */
void run_simulation(int channel_len,
                    int num_hosts,
                    const char host_name[],
                    const int host_pos[],
                    const int host_start[],
                    int frame_len,
                    int max_ticks);

#endif /* CSMA_CD_H */
