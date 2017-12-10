#include "Animation.h"

class StaticColor : public Animation
{
  public:
    StaticColor(uint32_t color, int start, int end);
    virtual uint32_t GenerateColor(int i, const PixelBuffer &pb);
    String GetDescription();
  private:
    uint32_t _color;
};
