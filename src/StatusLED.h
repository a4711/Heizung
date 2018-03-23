/*
 * StatusLED.h
 *
 *  Created on: 20.03.2018
 *      Author: a4711
 */

#ifndef SRC_STATUSLED_H_
#define SRC_STATUSLED_H_

#include <Arduino.h>
#include <Ticker.h>

class StatusLED
{
public:
  StatusLED();
  virtual ~StatusLED();

  enum ErrorType {
    Ok = 0,
    Setup = 1,
    WifiError = 3,
  };

  void setup(uint8_t x_outputPin)
  {
    outputPin = x_outputPin;
    pinMode(outputPin, OUTPUT);
    ticker.attach_ms(100, &s_tick, this);
  }

  static void s_tick(StatusLED* pThis)
  {
    pThis->tick();
  }

  void setError(ErrorType error)
  {
    if (error != currentError)
    {
      currentError = error;
      pulseWidth = 10;
      pulseCount = 1;
      sleep = 0;

      switch (currentError)
      {
        case Ok:
          break;
        case Setup:
          pulseWidth = 1;
          break;
        default:
          pulseWidth = 1;
          pulseCount = error;
          sleep = 10;
      }
    }
    load();
  }
private:

  void pulse(size_t numberOfPulses, unsigned long pulseWidth_ms, unsigned long separation_ms)
  {
    for (size_t n = 0; n < numberOfPulses; n++)
    {
      digitalWrite(outputPin, false);
      delay(pulseWidth_ms/2);
      digitalWrite(outputPin, true);
      delay(pulseWidth_ms/2);
    }
    delay(separation_ms);
  }
  
  void load()
  {
    currentTick = pulseCount * 2 * pulseWidth + sleep; // one pulse always need two ticks
  }
  
  void off()
  {
    digitalWrite(outputPin, true);
  }
  void on()
  {
    digitalWrite(outputPin, false);
  }

  void tick()
  {
   // return;
    if (!currentTick) load();
    if (currentTick > sleep)
    {
      size_t currentPulse = (currentTick - sleep) % (2*pulseWidth);
      if (currentPulse / pulseWidth)
      {
        on(); //second half of the pulse
      }
      else
      {
        off(); // first half of the pulse
      }
    }
    else
    {
      off();
    }
    
    currentTick--;
  }
  
  size_t pulseCount;  // number of pulses
  size_t pulseWidth;  // witdh of one pulse (one pulse needs two ticks)
  size_t sleep;       // number of sleep ticks.
  
  size_t currentTick;
  

  uint8_t   outputPin;
  ErrorType currentError;
  Ticker ticker;
};

#endif /* SRC_STATUSLED_H_ */
