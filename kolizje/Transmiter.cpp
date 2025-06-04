// Transmitter.cpp
#include "Transmitter.h"

Transmitter::Transmitter(const std::string& name, int position, const std::string& color)
    : name(name),
      position(position),
      attemptCounter(1),
      signalPositionL(position - 1),
      signalPositionR(position + 1),
      transmitting(false),
      delay(0),
      msgSent(false),
      color(color),
      waitTime(0),
      transmissionFailCounter(0)
{
}
