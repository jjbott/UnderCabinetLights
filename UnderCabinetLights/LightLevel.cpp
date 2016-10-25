#include "LightLevel.h"

LightLevel CalcLightLevelFromAnalog(int analogLightLevel)
{
  if ( analogLightLevel < 50 )
  {
    return LightLevel::Off;
  }
  else if ( analogLightLevel < 2000 )
  {
    return LightLevel::Low;
  }

  return LightLevel::High;
}
