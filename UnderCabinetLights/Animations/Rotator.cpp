#include "Rotator.h"
#include "Color.h"
#include "Config.h"
#include "Clock.h"

Rotator::Rotator(
  std::vector<std::shared_ptr<Animation>> animations,
  int start,
  int end,
  int rotateSeconds,
  int fadeSeconds)
  : Animation(start, end, false),
  _animations(std::move(animations)),
  _rotateSeconds{rotateSeconds},
  _fadeSeconds{fadeSeconds}
{
  _currentAnimation = _animations[0].get();
  _lastRotateTime = Clock::Millis();

  Particle.publish("DEBUG", "Rotator created");
}

void Rotator::UpdateFrame(ulong frame)
{
  Animation::UpdateFrame(frame);

  bool rotate =  Clock::TriggerEveryXMillis(_rotateSeconds*1000, _lastRotateTime) > 0;
  if ( rotate)
  {
    _currentAnimationIndex = (_currentAnimationIndex + 1) % _animations.size();
    _currentAnimation = _animations[_currentAnimationIndex].get();
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
