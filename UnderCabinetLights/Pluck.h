#include "Animation.h"
#pragma once

class Pluck : public Animation
{
  public:
    Pluck(uint32_t baseColor, int hueVariance, int durationMs, int start, int end, bool respectLightLevel);
    virtual uint32_t GenerateColor(int i, const PixelBuffer &pb);
    void UpdateFrame(ulong frame);
    String GetDescription();
  private:
    uint32_t _baseColor;
    uint16_t _hueVariance;
    float _newSparklePercent;
    int _durationMs;
    ulong _startFrame;
    double _currentCyclePosition; /* 0 <= x < 1 */
    uint32_t _currentColor;
    uint32_t _currentIntensity;
};
