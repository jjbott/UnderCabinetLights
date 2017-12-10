#pragma once

#include "Animation.h"
#include <vector>
#include <memory>

class Mirror : public Animation
{
  public:
    Mirror(std::shared_ptr<Animation> animation, String friendlyDescription = "");
    virtual uint32_t GenerateColor(int i, const PixelBuffer &pb);
    virtual void UpdateFrame(ulong frame);
    String GetDescription();
  protected:
    std::shared_ptr<Animation> _animation;
    std::unique_ptr<uint32_t[]> _mirroredColors;
};
