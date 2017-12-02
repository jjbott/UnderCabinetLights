#pragma once

#include "Animation.h"
#include <vector>
#include <memory>

class Decay : public Animation
{
  public:
    Decay(Animation* animation, double decayFactor);
    virtual uint32_t GenerateColor(int i, const PixelBuffer &pb);
    virtual void UpdateFrame(ulong frame);
    String GetDescription();
  protected:
    std::shared_ptr<Animation> _animation;
    double _decayFactor;
    ulong _lastFrame;
};
