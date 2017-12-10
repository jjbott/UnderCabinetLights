#include "Mirror.h"
#include "Color.h"

Mirror::Mirror(std::shared_ptr<Animation> animation, String friendlyDescription)
: Animation(animation->GetStart(), animation->GetEnd(), friendlyDescription),
  _animation{animation}
{
  _mirroredColors = std::unique_ptr<uint32_t[]>(new uint32_t[_animation->GetEnd() - _animation->GetStart()]);
}

void Mirror::UpdateFrame(ulong frame)
{
  Animation::UpdateFrame(frame);
  _animation->UpdateFrame(frame);
  std::fill(_mirroredColors.get(), _mirroredColors.get()+(_animation->GetEnd() - _animation->GetStart()), -1);
}

uint32_t Mirror::GenerateColor(int i, const PixelBuffer &pb)
{
  int mirroredIndex = _animation->GetEnd() - i + _animation->GetStart();
  int cacheIndex = i - _animation->GetStart();
  int renderIndex = i;
  if ( mirroredIndex < i )
  {
    // We were asked to render the mirrored portion
    renderIndex = mirroredIndex;
    cacheIndex = mirroredIndex - _animation->GetStart();
    cacheIndex = _animation->GetEnd() - i - _animation->GetStart();
  }

  if ((_mirroredColors)[cacheIndex] == -1)
  {
    _mirroredColors[cacheIndex] = _animation->GenerateColor(renderIndex, pb);
  }

  return _mirroredColors[cacheIndex];
}

String Mirror::GetDescription()
{
  if ( _friendlyDescription != "" ) return _friendlyDescription;

  return String("Mirroring: ") + _animation->GetDescription();
}
