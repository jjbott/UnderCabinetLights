#include "application.h"
#pragma once

class Clock
{
  // Similar to Timers, but, uh, I didnt know about Timers.
  // Plus provides microsecond resolution, and works regardless of interrupts
  // Also allows me to control at what point in the process the code is run
  // (no use calling something at 60fps if the colors havent been set up when called)
  public:
    static int TriggerEveryXMillis(unsigned long x, unsigned long &lastTriggeredMillis);
    static int TriggerEveryXMicros(unsigned long x, unsigned long &lastTriggeredMicros);
    static uint32_t ElapsedMicros(unsigned long now, unsigned long since);
    static uint32_t ElapsedMillis(unsigned long now, unsigned long since);
    static uint32_t Micros();
    static uint32_t Millis();
};
