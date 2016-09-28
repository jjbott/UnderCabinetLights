#include "Animation.h"

class StaticColor : public Animation
{
  public:
    StaticColor(uint32_t color, int start, int end, bool lightActivated);
    virtual uint32_t GenerateColor(ulong frame, int i, const PixelBuffer &pb);
    String GetDescription();
  private:
    uint32_t _color;
};
