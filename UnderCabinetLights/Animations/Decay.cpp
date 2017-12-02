#include "Decay.h"
#include "Color.h"

Decay::Decay(Animation* animation, double decayFactor)
: _animation{std::shared_ptr<Animation>(animation)},
  _decayFactor{decayFactor},
  Animation(animation->GetStart(), animation->GetEnd(), false)
{
  if ( _decayFactor < 0) _decayFactor = 0;
  if ( _decayFactor > 1) _decayFactor = 1;
}

void Decay::UpdateFrame(ulong frame)
{
  _lastFrame = _currentFrame;
  _animation->UpdateFrame(frame);
  Animation::UpdateFrame(frame);
}

uint32_t Decay::GenerateColor(int i, const PixelBuffer &pb)
{
  uint32_t color = _animation->GenerateColor(i, pb);
  if ( color != 0 )
  {
    return color;
  }
  return Color::Dim(pb.GetColor(i), _decayFactor);
}

String Decay::GetDescription()
{
  return String("Decaying: ") + _animation->GetDescription();
}