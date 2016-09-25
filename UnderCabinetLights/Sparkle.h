#include "Animation.h"

class Sparkle : public Animation
{
  public:
    Sparkle(uint32_t baseColor, int hueVariance, float newSparklePercent, int sparkleDurationMs, int start, int end);
    virtual uint32_t GenerateColor(ulong frame, int i, const PixelBuffer &pb);
    String GetDescription();
  private:
    uint32_t _baseColor;
    uint16_t _hueVariance;
    float _newSparklePercent;
    int _sparkleDurationMs;
    float _valueDecayPerFrame;
};
