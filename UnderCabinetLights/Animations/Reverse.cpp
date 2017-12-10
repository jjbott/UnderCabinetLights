#include "Reverse.h"
#include "Color.h"

Reverse::Reverse(std::shared_ptr<Animation> animation, String friendlyDescription)
:  Animation(animation->GetStart(), animation->GetEnd(), friendlyDescription),
 _animation{animation}
{
}

void Reverse::UpdateFrame(ulong frame)
{
  Animation::UpdateFrame(frame);
  _animation->UpdateFrame(frame);
}

uint32_t Reverse::GenerateColor(int i, const PixelBuffer &pb)
{
  int reversedIndex = _animation->GetEnd() - i + _animation->GetStart();
  //int reversedIndex = i;

  return _animation->GenerateColor(reversedIndex, pb);
}

String Reverse::GetDescription()
{
  if ( _friendlyDescription != "" ) return _friendlyDescription;

  return String("Reversing: ") + _animation->GetDescription();
}
