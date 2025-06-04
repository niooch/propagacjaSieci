// Transmitter.h
#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <string>

class Transmitter {
public:
    std::string name;
    int position;
    int attemptCounter;
    int signalPositionL;
    int signalPositionR;
    bool transmitting;
    int delay;
    bool msgSent;
    // (color field is no longer used, but we leave it here so constructors stay compatible)
    std::string color;
    int waitTime;
    int transmissionFailCounter;

    Transmitter(const std::string& name, int position, const std::string& color);
};

#endif // TRANSMITTER_H
