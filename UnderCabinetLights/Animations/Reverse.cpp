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

uint32_t Reverse::GenerateColor(int i, std::function<uint32_t(int)> colorLookup)
{
  int reversedIndex = _animation->GetEnd() - i + _animation->GetStart();
  auto reversedColorLookup = [&](int index) { return colorLookup(_animation->GetEnd() - index + _animation->GetStart()); };

  //int reversedIndex = i;

  return _animation->GenerateColor(reversedIndex, reversedColorLookup);
}

String Reverse::GetDescription()
{
  if ( _friendlyDescription != "" ) return _friendlyDescription;

  return String("Reversing: ") + _animation->GetDescription();
}
