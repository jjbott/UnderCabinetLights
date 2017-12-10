#include "application.h"
#include "PixelBuffer.h"
#pragma once

class PixelBuffer;

class Animation
{
  public:
    Animation(int start, int end, String friendlyDescription = "");
    bool IsObsolete();
    void Render(ulong frame, PixelBuffer &pb);
    virtual String GetDescription() = 0;

    // These are only public so animation wrappers can use them.
    // Maybe come up with a better way to do that
    virtual void UpdateFrame(ulong frame);
    virtual uint32_t GenerateColor(int i, std::function<uint32_t(int)> colorLookup ) = 0;

    int GetStart();
    int GetEnd();

  protected:

    int _start;
    int _end;

    ulong _currentFrame;

    String _friendlyDescription;
};
