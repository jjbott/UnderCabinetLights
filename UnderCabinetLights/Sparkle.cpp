#include "Sparkle.h"
#include "Config.h"
#include "Color.h"
#include "PixelBuffer.h"

Sparkle::Sparkle(
    std::vector<uint32_t> colors,
   int hueVariance, float newSparklePercent, int sparkleDurationMs, int start, int end, String friendlyDescription)
  :Animation(start, end, friendlyDescription),
  _colors(std::move(colors))
{
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

uint32_t Sparkle::GenerateColor(int i, std::function<uint32_t(int)> colorLookup)
{
  if ( (100 * ((double) rand() / (RAND_MAX))) < _newSparklePercent )
  {
    // new sparkle!!1
    int16_t h;
    uint8_t s;
    uint8_t v;
    uint32_t base_color = _colors[_colors.size() * ((double) rand() / (RAND_MAX))];

    Color::ColorToHsv(base_color, h, s, v); // should do this in constructor
    int16_t hue = h;
    if ( _hueVariance > 0 && _hueVariance <= 360)
    {
      hue = h + (rand() % (_hueVariance * 2) - _hueVariance);
    }
    return Color::HsvToColor(hue, s, v);
  }
  else
  {
    uint32_t color = colorLookup(i);
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
  if ( _friendlyDescription != "" ) return _friendlyDescription;

  return String::format("Sparkle: Base Color #%06X, Variance %d, Percent %f%%, Duration %dms, Start %d, End %d", _colors[0], (int)_hueVariance, _newSparklePercent, (int)_sparkleDurationMs, (int)_start, (int)_end);
}
