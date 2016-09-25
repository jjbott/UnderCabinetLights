#include "Sparkle.h"
#include "Config.h"
#include "Color.h"
#include "PixelBuffer.h"

Sparkle::Sparkle(uint32_t baseColor, int hueVariance, float newSparklePercent, int sparkleDurationMs, int start, int end)
  :Animation(start, end)
{
    _baseColor = baseColor;
    _hueVariance = hueVariance;
    _newSparklePercent = newSparklePercent;
    _sparkleDurationMs = sparkleDurationMs;
    // This will lose a lot of precision, and the result may be that the duration ends up no where near sparkleDuration.
    // But in the normal case it will be close enough
    _valueDecayPerFrame = 256 / (sparkleDurationMs * Config::FPS / 1000);
    if ( _valueDecayPerFrame <= 0 )
    {
      _valueDecayPerFrame = 1;
    }
}

uint32_t Sparkle::GenerateColor(ulong frame, int i, const PixelBuffer &pb)
{
  if ( (100 * ((double) rand() / (RAND_MAX))) < _newSparklePercent )
  {
    // new sparkle!!1
    int16_t h;
    uint8_t s;
    uint8_t v;
    Color::ColorToHsv(_baseColor, h, s, v); // should do this in constructor
    int16_t hue = h + (rand() % (_hueVariance * 2) - _hueVariance);
    return Color::HsvToColor(hue, s, v);
  }
  else
  {
    uint32_t color = pb.GetColor(i);
    if (color > 0)
    {
      int16_t h;
      uint8_t s;
      uint8_t v;
      Color::ColorToHsv(color, h, s, v); // should do this in constructor
      v -= _valueDecayPerFrame;
      v = v < 0 ? 0 : v;
      return Color::HsvToColor(h, s, v);
    }
    return color;
  }
}

String Sparkle::GetDescription()
{
  char buffer[100];
  snprintf(buffer, 100, "Sparkle: Base Color #%06X, Variance %d, Percent %f%%, Duration %dms, Start %d, End %d", _baseColor, (int)_hueVariance, _newSparklePercent, (int)_sparkleDurationMs, (int)_start, (int)_end);
  return String(buffer);
}
