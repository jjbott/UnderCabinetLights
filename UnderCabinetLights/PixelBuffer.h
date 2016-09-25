#define PIXEL_COUNT_STRIP1 43
#define PIXEL_COUNT_STRIP2 226
#define PIXEL_COUNT (PIXEL_COUNT_STRIP1 + PIXEL_COUNT_STRIP2)

#include "application.h"
#include "neopixel.h"

class PixelBuffer
{
  public:
    PixelBuffer();
    bool IsPixelDirty(int i);
    uint32_t GetColor(int i) const;
    void SetColor(int i, uint32_t color);
    bool Show();
  private:
    uint32_t _current[PIXEL_COUNT];
    uint32_t _next[PIXEL_COUNT];
    bool _nextDirty[PIXEL_COUNT];
    Adafruit_NeoPixel _strip1;
    Adafruit_NeoPixel _strip2;

    void SetStripPixelColor(int index, uint32_t color);


    bool _stripStatusPublished = false;
    ulong _lastStripStatusPublish = 0;
    void PublishLightStatus(bool changesMade);
};
