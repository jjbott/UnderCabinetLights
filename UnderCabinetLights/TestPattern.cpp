#include "TestPattern.h"
#include "Config.h"
#include "Color.h"
#include "PixelBuffer.h"

TestPattern::TestPattern(int start, int end, bool respectLightLevel)
  :Animation(start, end, respectLightLevel)
{
}

uint32_t TestPattern::GenerateColor(int i, const PixelBuffer &pb)
{
  int phase = (_currentFrame / Config::FPS) % 4;
  switch(phase)
  {
    case 0: return 0xFF0000;
    case 1: return 0x00FF00;
    case 2: return 0x0000FF;
    case 3: return 0xFFFFFF;
  }
}

String TestPattern     ::GetDescription()
{
  char buffer[100];
  snprintf(buffer, 100, "TestPattern: Start %d, End %d", (int)_start, (int)_end);
  return String(buffer);
}
