#pragma once

enum LightLevel
{
  Off = 0,
  Low = 1,
  High= 2
};

LightLevel CalcLightLevelFromAnalog(int analogLightLevel);
