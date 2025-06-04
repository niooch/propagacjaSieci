// Controller.cpp
#include "Controller.h"
#include <iostream>
#include <thread>
#include <chrono>

// Constructor: store references, seed RNG, clear network
Controller::Controller(std::vector<std::string>& network,
                       std::vector<Transmitter*>& transmitterList,
                       int printSpeed,
                       int delayRange)
    : network(network),
      transmitterList(transmitterList),
      arrayNoised(false),
      rng(std::random_device{}()),
      delayCounter(0),
      printSpeed(printSpeed),
      delayRange(delayRange),
      changeWaitTime(false)
{
    clearNetwork();
}

// Checks if the cell at transmitter->position is " " (empty)
bool Controller::checkIfPossibleTransmission(Transmitter* transmitter) {
    return network[transmitter->position] == " ";
}

// Checks if at this position there is something other than " " or "#"
bool Controller::isOtherTransmitting(Transmitter* transmitter) {
    const std::string& cell = network[transmitter->position];
    return !(cell == " " || cell == "#");
}

// Doubles attemptCounter, maybe prints “FAILED,” then recalculates waitTime
void Controller::waitTimeAdd(Transmitter* transmitter) {
    transmitter->attemptCounter *= 2;
    if (transmitter->attemptCounter > 1024) {
        if (transmitter->transmissionFailCounter == 6) {
            std::cout << "TRANSMISSION " << transmitter->name << " FAILED\n";
        }
        transmitter->transmissionFailCounter++;
    }
    std::uniform_int_distribution<int> dist(0, transmitter->attemptCounter - 1);
    int randomBackoff = dist(rng) + 1;
    transmitter->waitTime = randomBackoff * static_cast<int>(network.size());
}

// Assigns each transmitter a random initial delay in [0, delayRange-1]
void Controller::randomizeDelay() {
    std::uniform_int_distribution<int> dist(0, delayRange - 1);
    for (auto* trans : transmitterList) {
        trans->delay = dist(rng);
    }
}

// Returns true if every cell in network is "#"
bool Controller::checkIfArrayNoised() {
    for (const auto& cell : network) {
        if (cell != "#") {
            return false;
        }
    }
    return true;
}

// Clears the “message” of a transmitter after a collision or success
void Controller::startClearingMSG(Transmitter* transmitter) {
    // First, stop everyone from transmitting
    for (auto* t : transmitterList) {
        t->transmitting = false;
    }

    bool toRight = false;
    bool toLeft  = false;
    int mid = static_cast<int>(network.size()) / 2;
    if (transmitter->position > mid) {
        toRight = true;
    } else {
        toLeft = true;
    }

    // Clear one step at a time on each side
    if (transmitter->signalPositionR < static_cast<int>(network.size())) {
        if (toLeft) {
            network[transmitter->position] = " ";
        }
        network[transmitter->signalPositionR] = " ";
        transmitter->signalPositionR++;
    }
    if (transmitter->signalPositionL >= 0) {
        if (toRight) {
            network[transmitter->position] = " ";
        }
        network[transmitter->signalPositionL] = " ";
        transmitter->signalPositionL--;
    }

    // If both ends have fully cleared, reset msgSent and reposition signals
    if (transmitter->signalPositionL == -1 && transmitter->signalPositionR == static_cast<int>(network.size())) {
        transmitter->msgSent = false;
        transmitter->signalPositionR = transmitter->position + 1;
        transmitter->signalPositionL = transmitter->position - 1;
        std::uniform_int_distribution<int> dist(0, delayRange - 1);
        transmitter->delay = dist(rng) + delayCounter;
    }

    // If entire network is empty, just ensure msgSent is false again
    if (arrayClear()) {
        transmitter->msgSent = false;
    }
}

// Clears the “noised” network after everyone has turned into "#"
void Controller::startClearing(Transmitter* transmitter) {
    bool toRight = false;
    bool toLeft  = false;
    int mid = static_cast<int>(network.size()) / 2;
    if (transmitter->position > mid) {
        toRight = true;
    } else {
        toLeft = true;
    }

    // Stop everyone from transmitting
    for (auto* t : transmitterList) {
        t->transmitting = false;
    }

    // Clear one cell on each side
    if (transmitter->signalPositionR < static_cast<int>(network.size())) {
        if (toLeft) {
            network[transmitter->position] = " ";
        }
        network[transmitter->signalPositionR] = " ";
        transmitter->signalPositionR++;
    }
    if (transmitter->signalPositionL >= 0) {
        if (toRight) {
            network[transmitter->position] = " ";
        }
        network[transmitter->signalPositionL] = " ";
        transmitter->signalPositionL--;
    }

    // If both ends done, msgSent = false
    if (transmitter->signalPositionL == -1 && transmitter->signalPositionR == static_cast<int>(network.size())) {
        transmitter->msgSent = false;
    }

    // Once the entire array is " ", reset everything
    if (arrayClear()) {
        arrayNoised = false;
        if (!transmitter->msgSent) {
            std::uniform_int_distribution<int> dist(0, delayRange - 1);
            transmitter->delay = dist(rng) + delayCounter;
        }
        for (auto* t : transmitterList) {
            t->signalPositionR = t->position + 1;
            t->signalPositionL = t->position - 1;
            t->msgSent = false;
        }
    }
}

// Returns true if every cell in network is " " (empty)
bool Controller::arrayClear() {
    for (const auto& cell : network) {
        if (cell != " ") {
            return false;
        }
    }
    return true;
}

// Advances the message one step on each side if still transmitting
void Controller::continueTransmission(Transmitter* transmitter) {
    // Place the transmitter’s letter at its own position if empty
    if (transmitter->transmitting && network[transmitter->position] == " ") {
        network[transmitter->position] = transmitter->name;
    }

    // Send rightward
    if (transmitter->signalPositionR < static_cast<int>(network.size())) {
        sendMsgRight(transmitter);
    }
    // Send leftward
    if (transmitter->signalPositionL >= 0) {
        sendMsgLeft(transmitter);
    }

    // If both directions have gone off the ends, stop transmitting and reset
    if (transmitter->signalPositionL == -1 && transmitter->signalPositionR == static_cast<int>(network.size())) {
        transmitter->transmitting = false;
        transmitter->signalPositionL = transmitter->position - 1;
        transmitter->signalPositionR = transmitter->position + 1;
    }

    transmitter->msgSent = checkIfMsgSent(transmitter);
}

// Place a letter or "#" at signalPositionR
void Controller::sendMsgRight(Transmitter* transmitter) {
    int rp = transmitter->signalPositionR;
    if (network[rp] != " " && network[rp] != transmitter->name) {
        network[rp] = "#";
    } else {
        network[rp] = transmitter->name;
    }
    transmitter->signalPositionR++;
}

// Place a letter or "#" at signalPositionL
void Controller::sendMsgLeft(Transmitter* transmitter) {
    int lp = transmitter->signalPositionL;
    if (network[lp] != " " && network[lp] != transmitter->name) {
        network[lp] = "#";
    } else {
        network[lp] = transmitter->name;
    }
    transmitter->signalPositionL--;
}

// Print the entire network, with no spaces between cells, and "#" for collisions
void Controller::printNetwork() {
    for (const auto& cell : network) {
        // If it is " ", print a space
        if (cell == " ") {
            std::cout << " ";
        }
        // If it is "#", print "#"
        else if (cell == "#") {
            std::cout << "#";
        }
        // Otherwise it's a transmitter letter (e.g. "A", "B", "C")
        else {
            std::cout << cell;
        }
    }
    std::cout << "\n";
}

// Fill every slot in network with " "
void Controller::clearNetwork() {
    for (auto& cell : network) {
        cell = " ";
    }
}

// Returns true if every cell contains exactly transmitter->name
bool Controller::checkIfMsgSent(Transmitter* transmitter) {
    for (const auto& cell : network) {
        if (cell != transmitter->name) {
            return false;
        }
    }
    return true;
}

// The endless loop that mimics Controller.run() in Java
void Controller::run() {
    delayCounter = 1;
    randomizeDelay();

    while (true) {
        for (auto* transmitter : transmitterList) {
            bool failTransmission = false;

            if (transmitter->delay <= delayCounter) {
                if (arrayNoised) {
                    startClearing(transmitter);
                }
                else if (transmitter->msgSent) {
                    transmitter->attemptCounter = 1;
                    startClearingMSG(transmitter);
                }
                else if (checkIfArrayNoised()) {
                    arrayNoised = true;
                    changeWaitTime = true;
                }
                else if (transmitter->transmitting) {
                    if (transmitter->waitTime < 2) {
                        continueTransmission(transmitter);
                    } else {
                        transmitter->waitTime--;
                        transmitter->transmitting = false;
                    }
                }
                else if (checkIfPossibleTransmission(transmitter)) {
                    transmitter->transmitting = true;
                    transmitter->waitTime--;
                }
                else if (isOtherTransmitting(transmitter)) {
                    if (transmitter->waitTime < 2) {
                        failTransmission = true;
                    } else {
                        transmitter->waitTime--;
                    }
                }
            }

            if (failTransmission) {
                waitTimeAdd(transmitter);
            }
            else if (changeWaitTime) {
                changeWaitTime = false;
                for (auto* t : transmitterList) {
                    if (t->waitTime < 2) {
                        waitTimeAdd(t);
                    }
                }
            }
        }

        printNetwork();
        std::this_thread::sleep_for(std::chrono::milliseconds(printSpeed));
        delayCounter++;
    }
}
