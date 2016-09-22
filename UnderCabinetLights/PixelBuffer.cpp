#include "PixelBuffer.h"

PixelBuffer::PixelBuffer()
{
  for(int i = 0; i < PIXEL_COUNT; ++i)
  {
    _current[i] = _next[i] = 0;
    _nextDirty[i] = false;
  }
}

bool PixelBuffer::IsPixelDirty(int i)
{
  return _nextDirty[i];
}

uint32_t PixelBuffer::GetColor(int i) const
{
  // Assuming that anything querying colors is looking for the current color,
  // not the "next" color. We'll see how that pans out.
  return _current[i];
}

void PixelBuffer::SetColor(int i, uint32_t color)
{
  // Assuming that the pixel isnt dirty, or the caller knows what they're doing
  _next[i] = color;
  _nextDirty[i] = true;
}

void PixelBuffer::Show()
{
  for(int i = 0; i < PIXEL_COUNT; ++i)
  {
    if ( IsPixelDirty(i) )
    {
      //setColor(_next[i]);
      _current[i] = _next[i];
      _next[i] = 0;
      _nextDirty[i] = false;
    }
  }
}
