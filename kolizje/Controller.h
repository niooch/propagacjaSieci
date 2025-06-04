// Controller.h
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <vector>
#include <string>
#include <random>
#include "Transmitter.h"

class Controller {
public:
    Controller(std::vector<std::string>& network,
               std::vector<Transmitter*>& transmitterList,
               int printSpeed,
               int delayRange);

    // Runs the endless loop of checking transmissions, printing, and sleeping
    void run();

private:
    std::vector<std::string>& network;
    std::vector<Transmitter*>& transmitterList;
    bool arrayNoised;
    std::mt19937 rng;
    int delayCounter;
    int printSpeed;
    int delayRange;
    bool changeWaitTime;

    // Helper methods
    bool checkIfPossibleTransmission(Transmitter* transmitter);
    bool isOtherTransmitting(Transmitter* transmitter);
    void waitTimeAdd(Transmitter* transmitter);
    void randomizeDelay();
    bool checkIfArrayNoised();
    void startClearingMSG(Transmitter* transmitter);
    void startClearing(Transmitter* transmitter);
    bool arrayClear();
    void continueTransmission(Transmitter* transmitter);
    void sendMsgRight(Transmitter* transmitter);
    void sendMsgLeft(Transmitter* transmitter);
    void printNetwork();
    void clearNetwork();
    bool checkIfMsgSent(Transmitter* transmitter);
};

#endif // CONTROLLER_H
