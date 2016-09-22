#include "Animation.h"

class StaticColor : Animation
{
  public:
    StaticColor(uint32_t color, int start, int end);
    virtual uint32_t GenerateColor(int frame, int i, const PixelBuffer &pb);
  private:
    uint32_t _color;
};
