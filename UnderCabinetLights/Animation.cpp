#include "Animation.h"
#include "PixelBuffer.h"

Animation::Animation(int start, int end, bool respectLightLevel)
{
  _start = start;
  _end = end;
  _respectLightLevel = respectLightLevel;
}

int Animation::GetStart()
{
  return _start;
}

int Animation::GetEnd()
{
  return _end;
}

bool Animation::IsObsolete()
{
  // I'll mess up _start and _end when a Render cycle doesnt actually render anything
  return _start > _end;
}

void Animation::UpdateFrame(ulong frame)
{
  _currentFrame = frame;
}

void Animation::Render(ulong frame, LightLevel lightLevel, PixelBuffer &pb)
{
  int i = _start;
  while(pb.IsPixelDirty(i))
  {
    i = ++_start;
  }

  UpdateFrame(frame);

  int lastRenderedIndex = -1;
  for(i = _start; i <= _end; ++i)
  {
    // Only render to pixels that someone else didnt get to first
    if ( !pb.IsPixelDirty(i) )
    {
      uint32_t color = GenerateColor(i, pb);
      if ( _respectLightLevel )
      {
        if ( lightLevel == LightLevel::Off)
        {
          color = 0;
        }
        else if ( lightLevel == LightLevel::Low )
        {
          float dimAmount = 0.5;
          uint8_t r = color >> 16;
          uint8_t g = color >> 8;
          uint8_t b = color;
          r *= dimAmount;
          g *= dimAmount;
          b *= dimAmount;
          color = Adafruit_NeoPixel::Color(r,g,b);
        }
      }
      pb.SetColor(i, color);
      lastRenderedIndex = i;
    }
  }
  _end = lastRenderedIndex;
}
