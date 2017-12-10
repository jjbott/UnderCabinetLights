#include "Rotator.h"
#include "Color.h"
#include "Config.h"
#include "Clock.h"

Rotator::Rotator(
  std::vector<std::shared_ptr<Animation>> animations,
  int start,
  int end,
  int rotateSeconds,
  int fadeSeconds,
  bool random)
  : Animation(start, end),
  _animations(std::move(animations)),
  _rotateSeconds{rotateSeconds},
  _fadeSeconds{fadeSeconds},
  _random{random}
{
  _currentAnimation = _animations[0].get();
  _lastRotateTime = Clock::Now();

  Particle.publish("DEBUG", "Rotator created");
}

void Rotator::UpdateFrame(ulong frame)
{
  Animation::UpdateFrame(frame);

  bool rotate =  Clock::TriggerEveryXSeconds(_rotateSeconds, _lastRotateTime) > 0;
  if ( rotate)
  {
    if ( _random && (_animations.size() > 1))
    {
      int nextAnimationIndex = _currentAnimationIndex;
      while(nextAnimationIndex == _currentAnimationIndex)
      {
        nextAnimationIndex = _animations.size() * ((double) rand() / (RAND_MAX));
      }
      _currentAnimationIndex = nextAnimationIndex;
    }
    else
    {
      _currentAnimationIndex = (_currentAnimationIndex + 1) % _animations.size();
    }
    _currentAnimation = _animations[_currentAnimationIndex].get();

    Particle.publish("DEBUG", String("Rotated to: ") + _currentAnimation->GetDescription());
  }

  _currentAnimation->UpdateFrame(frame);
}

uint32_t Rotator::GenerateColor(int i, const PixelBuffer &pb)
{
  return _currentAnimation->GenerateColor(i, pb);
}

String Rotator::GetDescription()
{
  return _currentAnimation->GetDescription();
}
