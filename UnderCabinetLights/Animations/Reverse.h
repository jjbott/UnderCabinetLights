#pragma once

#include "Animation.h"
#include <vector>
#include <memory>

class Reverse : public Animation
{
  public:
    Reverse(std::shared_ptr<Animation> animation, String friendlyDescription = "");
    virtual uint32_t GenerateColor(int i, std::function<uint32_t(int)> colorLookup);
    virtual void UpdateFrame(ulong frame);
    String GetDescription();
  protected:
    std::shared_ptr<Animation> _animation;
};
