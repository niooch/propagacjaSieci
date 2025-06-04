#include <iostream>
#include <vector>
#include <string>
#include "Transmitter.h"
#include "Controller.h"

// ANSI color codes (same as Java’s \u001B[XXm)
const std::string BLACK  = "\033[30m";
const std::string GREEN  = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE   = "\033[34m";
const std::string PURPLE = "\033[35m";
const std::string CYAN   = "\033[36m";
const std::string WHITE  = "\033[37m";
const std::string RESET  = "\033[0m";

int main() {
    const int networkLength = 100;
    const int printSpeed    = 40;   // milliseconds between prints
    const int delayRange    = 100;  // range for initial random delays

    // Create a network of length 60, all initialized to "0"
    std::vector<std::string> network(networkLength, "0");

    // Instantiate three Transmitters exactly as in Java
    Transmitter t1("A", 30, YELLOW);
    Transmitter t2("B", 10, CYAN);
    Transmitter t3("C", 50, GREEN);

    // Collect pointers to them in a vector
    std::vector<Transmitter*> transmitters;
    transmitters.push_back(&t1);
    transmitters.push_back(&t2);
    transmitters.push_back(&t3);

    // Instantiate the controller and start the run loop
    Controller ctrl(network, transmitters, printSpeed, delayRange);
    ctrl.run();  // this never returns

    return 0;
}

