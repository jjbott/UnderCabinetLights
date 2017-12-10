#pragma once

#include "Animation.h"
#include <vector>
#include <memory>

class Decay : public Animation
{
  public:
    Decay(Animation* animation, double decayFactor, String friendlyDescription = "");
    virtual uint32_t GenerateColor(int i, std::function<uint32_t(int)> colorLookup);
    virtual void UpdateFrame(ulong frame);
    String GetDescription();
  protected:
    std::shared_ptr<Animation> _animation;
    double _decayFactor;
    ulong _lastFrame;
};
