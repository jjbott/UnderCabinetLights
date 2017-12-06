#include "application.h"
#include "Clock.h"

const unsigned long MAX_MICROS = 35791394;

unsigned long Clock::ElapsedMicros(unsigned long now, unsigned long since)
{
  unsigned long elapsed = now - since;
  if ( now < since )
  {
    // time overflowed, fixed elapsed time
    elapsed = (MAX_MICROS - since) + now;
  }

  return elapsed;
}

unsigned long Clock::ElapsedMillis(unsigned long now, unsigned long since)
{
  return ElapsedMicros(now * 1000, since * 1000) / 1000;
}

bool Clock::TriggerInXSeconds(ulong x, ulong startTime)
{
  return (Time.now() - startTime) > x;
}

int Clock::TriggerEveryXSeconds(ulong x, ulong& lastTriggeredTime)
{
  ulong now = Now();
  ulong elapsed = now - lastTriggeredTime;
  if ( elapsed > x )
  {
    // I'm assuming that this is called often enough that using "now" will never be super wrong
    lastTriggeredTime = now;
    return elapsed / x;
  }
  return 0;
}

int Clock::TriggerEveryXMillis(unsigned long x, unsigned long &lastTriggeredMicros)
{
  return Clock::TriggerEveryXMicros(x * 1000, lastTriggeredMicros);
}

int Clock::TriggerEveryXMicros(ulong x, ulong &lastTriggeredMicros)
{
  ulong now = Micros();
  ulong elapsed = ElapsedMicros(now, lastTriggeredMicros);
  if ( elapsed > x )
  {
    // Dont set last time to "now".
    // It's pretty likely that elapsed is greater than x.
    // That means that if we set the last time to "now" every time, we're accumulating error.
    // Instead make sure the last time is on a x "boundary"? yeah.
    lastTriggeredMicros = (lastTriggeredMicros + elapsed - (elapsed % x)) % MAX_MICROS;
    return elapsed / x;
  }
  return 0;
}

uint32_t Clock::Micros()
{
  return DWT->CYCCNT / SYSTEM_US_TICKS;
}

uint32_t Clock::Millis()
{
  // Max is 35790
  return Micros() / 1000;
}

uint32_t Clock::Now()
{
  return Time.now();
}
