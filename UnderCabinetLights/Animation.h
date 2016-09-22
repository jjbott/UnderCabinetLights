#include "application.h"

class PixelBuffer;

class Animation
{
  public:
    Animation(int start, int end);
    bool IsObsolete();
  protected:
    void Render(uint32_t frame, PixelBuffer &pb);
    virtual uint32_t GenerateColor(uint32_t frame, int i, const PixelBuffer &pb) = 0;
    int _start;
    int _end;
};
