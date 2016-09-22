#define PIXEL_COUNT_STRIP1 43
#define PIXEL_COUNT_STRIP2 226
#define PIXEL_COUNT (PIXEL_COUNT_STRIP1 + PIXEL_COUNT_STRIP2)

#include "application.h"

class PixelBuffer
{
  public:
    PixelBuffer();
    bool IsPixelDirty(int i);
    uint32_t GetColor(int i) const;
    void SetColor(int i, uint32_t color);
    void Show();
  private:
    uint32_t _current[PIXEL_COUNT];
    uint32_t _next[PIXEL_COUNT];
    bool _nextDirty[PIXEL_COUNT];
};
