#include "StaticColor.h"

StaticColor::StaticColor(uint32_t color, int start, int end)
  :Animation(start, end)
{
    _color = color;
}

uint32_t StaticColor::GenerateColor(int i, std::function<uint32_t(int)> colorLookup)
{
  return _color;
}

String StaticColor::GetDescription()
{
  char buffer[100];
  snprintf(buffer, 100, "StaticColor: Color #%06X, Start %d, End %d", _color, (int)_start, (int)_end);
  return String(buffer);
}
