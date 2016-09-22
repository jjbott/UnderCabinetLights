#include "Rainbow.h"
#include "Config.h"
#include "Color.h"
#include "math.h"

Rainbow::Rainbow(uint32_t startingFrame, int start, int end, int cycleDurationMs, int width)
  :Animation(start, end)
{
  _startingFrame = startingFrame;
  _cycleDurationFrames = cycleDurationMs * Config::FPS / 1000.0;
  _width = width;
}

uint16_t Rainbow::CalculateStep(int frame)
{
  return fmod((frame - _startingFrame), _cycleDurationFrames) * 360 / _cycleDurationFrames;
}

uint32_t Rainbow::GenerateColor(int frame, int i, const PixelBuffer &pb)
{
  uint16_t step = CalculateStep(frame);
  return Color::HsvToColor((i * 360 / _width) + step, 255, 255);
}
