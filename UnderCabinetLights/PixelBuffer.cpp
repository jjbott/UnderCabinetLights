#include "PixelBuffer.h"
#include "Clock.h"
#include "Config.h"

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D6
#define PIXEL_TYPE WS2812B2
#define PIXEL_PIN_STRIP2 D5

// LEDs per segment
// 53
// 86
// 29
// 28
// 30
// 43


PixelBuffer::PixelBuffer()
  : _strip1(PIXEL_COUNT_STRIP1, PIXEL_PIN, PIXEL_TYPE),
  _strip2(PIXEL_COUNT_STRIP2, PIXEL_PIN_STRIP2, PIXEL_TYPE)
{
  for(int i = 0; i < PIXEL_COUNT; ++i)
  {
    _current[i] = _next[i] = 0;
    _nextDirty[i] = false;
  }

  _strip1.begin();
  _strip2.begin();

}

bool PixelBuffer::IsPixelDirty(int i)
{
  return _nextDirty[i];
}

uint32_t PixelBuffer::GetColor(int i) const
{
  // Assuming that anything querying colors is looking for the current color,
  // not the "next" color. We'll see how that pans out.
  //return _current[i];

  if ( i < _strip2.numPixels() )
  {
      return _strip2.getPixelColor(_strip2.numPixels() - i - 1);
  }
  else
  {
      return _strip1.getPixelColor(i - _strip2.numPixels());
  }
}

void PixelBuffer::SetColor(int i, uint32_t color)
{
  // Assuming that the pixel isnt dirty, or the caller knows what they're doing
  _next[i] = color;
  _nextDirty[i] = true;
}

bool PixelBuffer::Show()
{
  bool madeChanges = false;
  for(int i = 0; i < PIXEL_COUNT; ++i)
  {
    if ( IsPixelDirty(i) )
    {
      madeChanges = true;
      SetStripPixelColor(i, _next[i]);
      //_current[i] = _next[i];
      _next[i] = 0;
      _nextDirty[i] = false;
    }
  }

  if ( madeChanges )
  {
    _strip1.show();
    _strip2.show();
  }

  PublishLightStatus(madeChanges);

  return madeChanges;
}

void PixelBuffer::SetStripPixelColor(int index, uint32_t color)
{
    if ( index < PIXEL_COUNT_STRIP2 )
    {
        _strip2.setPixelColor(PIXEL_COUNT_STRIP2 - index - 1, color);
    }
    else
    {
        _strip1.setPixelColor(index - PIXEL_COUNT_STRIP2, color);
    }
}


// This might not be the best home for this code, but it works
void PixelBuffer::PublishLightStatus(bool changesMade)
{
  // Ensure any changes get published eventually
  if ( !_stripStatusPublished || changesMade)
  {
    _stripStatusPublished = false;
    if ( Clock::TriggerEveryXMillis(500, _lastStripStatusPublish) )
    {
      int count = PIXEL_COUNT;

      byte buffer[1 + (count * 3)];
      buffer[0] = 's';

      for(int i = 0; i < count; ++i)
      {
        uint32_t color = PixelBuffer::GetColor(i);
        buffer[1+ (i*3)] = (char)(color >> 16);
        buffer[1+ (i*3)+1] = (char)(color >> 8);
        buffer[1+ (i*3)+2] = (char)color;
      }

      UDP Udp;
      Udp.begin(123);
      Udp.sendPacket(buffer, sizeof(buffer), Config::PublishToIp, Config::PublishToPort);
      _stripStatusPublished = true;
    }
  }
}
