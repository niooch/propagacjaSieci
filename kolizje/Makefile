# Makefile dla programu CSMA/CD
# Pliki źródłowe: main.c, csma_cd.c
# Nagłówek:       csma_cd.h
# Kompilator:     gcc (C11), włączone ostrzeżenia -Wall

# Nazwa pliku wykonywalnego
TARGET = csma_cd_sim

# Kompilator i flagi
CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2

# Lista obiektów
OBJS = main.o csma_cd.o

# Domyślny cel: buduje program
all: $(TARGET)

# Reguła łączenia obiektów w plik wykonywalny
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Kompilacja main.c (zależne od nagłówka csma_cd.h)
main.o: main.c csma_cd.h
	$(CC) $(CFLAGS) -c main.c

# Kompilacja csma_cd.c (zależne od nagłówka csma_cd.h)
csma_cd.o: csma_cd.c csma_cd.h
	$(CC) $(CFLAGS) -c csma_cd.c

# Cel czyszczący (usuwa pliki obiektów i binarkę)
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

# Możliwość ręcznego uruchamiania
.PHONY: run
run: $(TARGET)
	./$(TARGET)

