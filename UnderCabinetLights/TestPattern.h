#include "Animation.h"

class TestPattern : public Animation
{
  public:
    TestPattern(int start, int end);
    virtual uint32_t GenerateColor(int i, const PixelBuffer &pb);
    String GetDescription();
  private:
    uint32_t _step;
};
