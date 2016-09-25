class Clock
{
  // Similar to Timers, but, uh, I didnt know about Timers.
  // Plus provides microsecond resolution, and works regardless of interrupts
  // Also allows me to control at what point in the process the code is run
  // (no use calling something at 60fps if the colors havent been set up when called)
  public:
    static bool TriggerEveryXMillis(unsigned long x, unsigned long &lastTriggeredMillis);
    static bool TriggerEveryXMicros(unsigned long x, unsigned long &lastTriggeredMicros);
};
