#include "Animation.h"

class TestPattern : public Animation
{
  public:
    TestPattern(int start, int end, bool respectLightLevel);
    virtual uint32_t GenerateColor(ulong frame, int i, const PixelBuffer &pb);
    String GetDescription();
  private:
    uint32_t _step;
};
