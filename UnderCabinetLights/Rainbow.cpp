#include "Rainbow.h"
#include "Config.h"
#include "Color.h"
#include "math.h"

Rainbow::Rainbow(uint32_t startingFrame, int start, int end, int cycleDurationMs, int width)
  :Animation(start, end)
{
  _startingFrame = startingFrame;
  _cycleDurationMs = cycleDurationMs;
  _cycleDurationFrames = cycleDurationMs * Config::FPS / 1000.0;
  _width = width;
}

uint16_t Rainbow::CalculateStep(int frame)
{
  return fmod((frame - _startingFrame), _cycleDurationFrames) * 360 / _cycleDurationFrames;
}

uint32_t Rainbow::GenerateColor(int i, const PixelBuffer &pb)
{
  uint16_t step = CalculateStep(_currentFrame);
  return Color::HsvToColor((i * 360 / _width) + step, 255, 255);
}

String Rainbow::GetDescription()
{
  char buffer[100];
  snprintf(buffer, 100, "Rainbow: Start Frame #%d, Duration %d, Width %d, Start %d, End %d", (int)_startingFrame, (int)_cycleDurationMs, (int)_width, (int)_start, (int)_end);
  return String(buffer);
}
