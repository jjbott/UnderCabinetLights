#include "Animation.h"

class Sparkle : Animation
{
  public:
    Sparkle(uint32_t baseColor, int hueVariance, float newSparklePercent, int sparkleDurationMs, int start, int end);
    virtual uint32_t GenerateColor(int frame, int i, const PixelBuffer &pb);
  private:
    uint32_t _baseColor;
    uint16_t _hueVariance;
    float _newSparklePercent;
    float _valueDecayPerFrame;
};
