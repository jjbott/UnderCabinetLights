#include "Animation.h"

class Rainbow : Animation
{
  public:
    Rainbow(uint32_t startingFrame, int start, int end, int cycleDurationMs, int width);
    virtual uint32_t GenerateColor(int frame, int i, const PixelBuffer &pb);
  private:
    uint16_t CalculateStep(int frame);
    uint32_t _startingFrame;
    double _cycleDurationFrames;
    int _width;
};
