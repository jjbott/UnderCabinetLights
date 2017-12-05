#include "Animation.h"
#include <vector>

class Rotator : public Animation
{
  public:
    Rotator(std::vector<std::shared_ptr<Animation>> animations,
    int start,
    int end,
    int rotateSeconds,
    int fadeSeconds,
    bool random = false);
    virtual uint32_t GenerateColor(int i, const PixelBuffer &pb);
    void UpdateFrame(ulong frame);
    String GetDescription();
  private:
    std::vector<std::shared_ptr<Animation>> _animations;
    int _rotateSeconds;
    int _fadeSeconds;
    bool _random;
    int _currentAnimationIndex = 0;
    Animation* _currentAnimation = 0;
    ulong _lastRotateTime = 0;
    double _currentOffset = 0;
};
