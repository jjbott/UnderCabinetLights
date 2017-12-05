#include "Animation.h"
#include <vector>

class Sparkle : public Animation
{
  public:
    Sparkle(std::vector<uint32_t> colors, int hueVariance, float newSparklePercent, int sparkleDurationMs, int start, int end, bool respectLightLevel);
    virtual uint32_t GenerateColor(int i, const PixelBuffer &pb);
    String GetDescription();
  private:
    std::vector<uint32_t> _colors;
    uint16_t _hueVariance;
    float _newSparklePercent;
    int _sparkleDurationMs;
    float _valueDecayPerFrame;
};
