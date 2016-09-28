#include "StaticColor.h"

StaticColor::StaticColor(uint32_t color, int start, int end, bool respectLightLevel)
  :Animation(start, end, respectLightLevel)
{
    _color = color;
}

uint32_t StaticColor::GenerateColor(ulong frame, int i, const PixelBuffer &pb)
{
  return _color;
}

String StaticColor::GetDescription()
{
  char buffer[100];
  snprintf(buffer, 100, "StaticColor: Color #%06X, Start %d, End %d", _color, (int)_start, (int)_end);
  return String(buffer);
}
