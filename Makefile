# Makefile for bitcrc_encode and bitcrc_decode

CXX       := g++
CXXFLAGS  := -std=c++11 -O2 -Wall
TARGETS   := bitcrc_encode bitcrc_decode
SRCS      := bitcrc_encode.cpp bitcrc_decode.cpp

.PHONY: all clean

all: $(TARGETS)

bitcrc_encode: bitcrc_encode.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

bitcrc_decode: bitcrc_decode.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) *.o
