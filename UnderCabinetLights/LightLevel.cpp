#include "LightLevel.h"
LightLevel lastLightLevel = LightLevel::Off;
bool inTransition = false;

const int OFF_THRESHOLD = 10;
const int HIGH_THRESHOLD = 2160;
const int LOW_THRESHOLD = 1500;
LightLevel CalcLightLevelFromAnalog(int analogLightLevel)
{
  // The incandescent light is slow.
  // Trigger the next mode as soon as we detect a change.
  // This helps with the timing of secret knocks. We better detect when the
  // button was pressed instead of when the light level fully changed.
  if ( !inTransition )
  {
    if (lastLightLevel == LightLevel::Off && analogLightLevel > OFF_THRESHOLD)
    {
      lastLightLevel = LightLevel::High;
      inTransition = true;
    }
    else if (lastLightLevel == LightLevel::High && analogLightLevel < HIGH_THRESHOLD )
    {
      lastLightLevel = LightLevel::Low;
      inTransition = true;
    }
    else if (lastLightLevel == LightLevel::Low && analogLightLevel < LOW_THRESHOLD )
    {
      lastLightLevel = LightLevel::Off;
      inTransition = true;
    }
  }

  if ( inTransition )
  {
    if ( (lastLightLevel == LightLevel::Off && analogLightLevel <= OFF_THRESHOLD)
        || (lastLightLevel == LightLevel::High && analogLightLevel >= HIGH_THRESHOLD)
        || (lastLightLevel == LightLevel::Low && analogLightLevel >= LOW_THRESHOLD) )
    {
      inTransition = false;
    }
  }

  return lastLightLevel;
}
