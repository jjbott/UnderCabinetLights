#include "Animation.h"

class Rainbow : public Animation
{
  public:
    Rainbow(uint32_t startingFrame, int start, int end, int cycleDurationMs, int width, bool respectLightLevel);
    uint32_t GenerateColor(ulong frame, int i, const PixelBuffer &pb);
    String GetDescription();
  private:
    uint16_t CalculateStep(int frame);
    uint32_t _startingFrame;
    int _cycleDurationMs;
    double _cycleDurationFrames;
    int _width;
};
