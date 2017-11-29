#include "Pluck.h"
#include "Config.h"
#include "Color.h"
#include "PixelBuffer.h"
#include <math.h>

Pluck::Pluck(uint32_t baseColor, int hueVariance, int durationMs, int start, int end, bool respectLightLevel)
  :Animation(start, end, respectLightLevel)
{
    _baseColor = baseColor;
    _hueVariance = hueVariance;
    _durationMs = durationMs;
    _startFrame = 0xFFFFFFFF;
    _currentFrame = 0xFFFFFFFF;
    _currentCyclePosition = 1; // Force a new cycle
    _currentColor = _baseColor;
}

uint32_t Pluck::GenerateColor(ulong frame, int i, const PixelBuffer &pb)
{
  if ( _startFrame == 0xFFFFFFFF )
  {
    _startFrame = frame;
  }

  if ( _currentFrame != frame )
  {
    _currentFrame = frame;
    // recalc... something
    ulong frameDiff = abs(_currentFrame - _startFrame);
    double framesPerCycle = _durationMs * Config::FPS / 1000.0;
    double cyclePosition = (frameDiff / framesPerCycle) - floor(frameDiff / framesPerCycle);
    if ( cyclePosition < _currentCyclePosition )
    {
      // started new cycle. Pick new color.  Stole code from Sparkle. Functionify it in Color someday. Thanks.
      int16_t h;
      uint8_t s;
      uint8_t v;
      Color::ColorToHsv(_baseColor, h, s, v);
      int16_t hue = h + (rand() % (_hueVariance * 2) - _hueVariance);
      _currentColor = Color::HsvToColor(hue, s, v);
    }
    _currentCyclePosition = cyclePosition;

    _currentIntensity = 255.0 * pow(2.7182818284, -2.624 * _currentCyclePosition);
  }

  int16_t h;
  uint8_t s;
  uint8_t v;
  Color::ColorToHsv(_currentColor, h, s, v);
  int newValue = map(_currentIntensity, 0, 255, 0, v);
  return Color::HsvToColor(h, s, newValue);
}

String Pluck::GetDescription()
{
  char buffer[100];
  snprintf(buffer, 100, "Pluck: Base Color #%06X, Variance %d, Duration %dms, Start %d, End %d", _baseColor, (int)_hueVariance, (int)_durationMs, (int)_start, (int)_end);
  return String(buffer);
}