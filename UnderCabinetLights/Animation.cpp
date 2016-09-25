#include "Animation.h"
#include "PixelBuffer.h"

Animation::Animation(int start, int end)
{
  _start = start;
  _end = end;
}

bool Animation::IsObsolete()
{
  // I'll mess up _start and _end when a Render cycle doesnt actually render anything
  return _start > _end;
}

void Animation::Render(ulong frame, PixelBuffer &pb)
{
  int i = _start;
  while(pb.IsPixelDirty(i))
  {
    i = ++_start;
  }

  int lastRenderedIndex = -1;
  for(i = _start; i <= _end; ++i)
  {
    // Only render to pixels that someone else didnt get to first
    if ( !pb.IsPixelDirty(i) )
    {
      pb.SetColor(i, GenerateColor(frame, i, pb));
      lastRenderedIndex = i;
    }
  }
  _end = lastRenderedIndex;
}
