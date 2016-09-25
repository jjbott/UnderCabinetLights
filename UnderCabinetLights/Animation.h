#include "application.h"
#pragma once

class PixelBuffer;

class Animation
{
  public:
    Animation(int start, int end);
    bool IsObsolete();
    void Render(ulong frame, PixelBuffer &pb);
    virtual String GetDescription() = 0;
  protected:
    virtual uint32_t GenerateColor(ulong frame, int i, const PixelBuffer &pb) = 0;
    int _start;
    int _end;
};
