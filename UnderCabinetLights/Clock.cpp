#include "Clock.h"
#include "application.h"
/*
unsigned long Clock::ElapsedMillis(unsigned long now, unsigned long since)
{
  unsigned long elapsed = now - since;
  if ( now < since )
  {
    // time overflowed, fixed elapsed time
    elapsed = (ULONG_MAX - since) + now;
  }

  return elapsed;
}
*/
unsigned long elapsedMicros(unsigned long now, unsigned long since)
{
  const unsigned long MAX_MICROS = ULONG_MAX; //35791394;
  unsigned long elapsed = now - since;
  if ( now < since )
  {
    // time overflowed, fixed elapsed time
    elapsed = (MAX_MICROS - since) + now;
  }

  return elapsed;
}

bool Clock::TriggerEveryXMillis(unsigned long x, unsigned long &lastTriggeredMillis)
{
  return Clock::TriggerEveryXMicros(x * 1000, lastTriggeredMillis);
/*
  ulong now = DWT->CYCCNT / SYSTEM_US_TICKS;
  ulong elapsed = elapsedMicros(now, lastTriggeredMillis) / 1000;
  if ( elapsed > x )
  {
    lastTriggeredMillis = now;
    return true;
  }
  return false;
  */
}

bool Clock::TriggerEveryXMicros(ulong x, ulong &lastTriggeredMicros)
{
  ulong now = DWT->CYCCNT / SYSTEM_US_TICKS;
  ulong elapsed = elapsedMicros(now, lastTriggeredMicros);
  if ( elapsed > x )
  {
    // Dont set last time to "now".
    // It's pretty likely that elapsed is greater than x.
    // That means that if we set the last time to "now" every time, we're accumulating error.
    // Instead make sure the last time is on a x "boundary"? yeah.
    lastTriggeredMicros += elapsed - (elapsed % x);
    return true;
  }
  return false;
}
