/*
 * StatusLED.cpp
 *
 *  Created on: 20.03.2018
 *      Author: a4711
 */

#include "StatusLED.h"

StatusLED::StatusLED (): pulseCount(1), pulseWidth(1), sleep(0), currentTick(0),
  outputPin(0), currentError(Ok)
{
}

StatusLED::~StatusLED ()
{
}

