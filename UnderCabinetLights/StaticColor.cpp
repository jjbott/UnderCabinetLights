#include "StaticColor.h"

StaticColor::StaticColor(uint32_t color, int start, int end)
  :Animation(start, end)
{
    _color = color;
}

uint32_t StaticColor::GenerateColor(int frame, int i, const PixelBuffer &pb)
{
  return _color;
}
