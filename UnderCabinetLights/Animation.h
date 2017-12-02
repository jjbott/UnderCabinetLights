#include "application.h"
#include "LightLevel.h"
#include "PixelBuffer.h"
#pragma once

class PixelBuffer;

class Animation
{
  public:
    Animation(int start, int end, bool respectLightLevel);
    bool IsObsolete();
    void Render(ulong frame, LightLevel LightLevel, PixelBuffer &pb);
    virtual String GetDescription() = 0;

    // These are only public so animation wrappers can use them.
    // Maybe come up with a better way to do that
    virtual void UpdateFrame(ulong frame);
    virtual uint32_t GenerateColor(int i, const PixelBuffer &pb) = 0;

    int GetStart();
    int GetEnd();

  protected:

    int _start;
    int _end;
    bool _respectLightLevel;

    ulong _currentFrame;
};
