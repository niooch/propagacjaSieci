# Propagacja sieci
Rozwiązania zadań z 3 listy z Technologii Sieciowych.

## Zadania
1. Napisz program ramkujący zgodnie z zasadą "rozpychania bitów" (podaną na wykładzie), oraz weryfikujacy poprawność ramki metodą CRC . Program ma odczytywać pewien źródłowy plik tekstowy 'Z' zawierający dowolny ciąg złożony ze znaków '0' i '1' (symulujacy strumień bitów) i zapisywać ramkami odpowiednio sformatowany ciąg do inngo pliku tekstowego 'W'. Program powinien obliczać i wstawiać do ramki pola kontrolne CRC - formatowane za pomocą ciągów złożonych ze znaków '0' i '1'. Napisz program, realizujacy procedure odwrotną, tzn. który odzczytuje plik wynikowy 'W' i dla poprawnych danych CRC przepisuje jego zawartość tak, aby otrzymać kopię oryginalnego pliku źródłowego 'Z'.
2. Napisz program (grupę programów) do symulowania ethernetowej metody dostepu do medium transmisyjnego (CSMA/CD). Wspólne łącze realizowane jest za pomocą tablicy: propagacja sygnału symulowana jest za pomoca propagacji wartości do sąsiednich komórek. Zrealizuj ćwiczenie tak, aby symulacje można było w łatwy sposób testować i aby otrzymane wyniki były łatwe w interpretacji.

### Odpalenie
Jest make file
```bash
make
make bitcrc_encode
make bitcrc_decode
```
### Bit Stuffing -- Zadanie 1.
Polega na dodaniu dodatkowych bitów do strumienia danych, aby uniknąć sytuacji, w której ciąg bitów mógłby być interpretowany jako specjalny znacznik ramki. W przypadku tego zadania, program będzie dodawał bity '0' po każdym ciągu pięciu kolejnych bitów '1'.


### Algorytm CSMA/CD
