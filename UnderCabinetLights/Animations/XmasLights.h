#include "Animation.h"
#include <vector>

class XmasLights : public Animation
{
  public:
    XmasLights(std::vector<uint32_t> colors,
      int start,
      int end,
      int speed,
      int distance,
      String friendlyDescription = "");
    virtual uint32_t GenerateColor(int i, const PixelBuffer &pb);
    void UpdateFrame(ulong frame);
    String GetDescription();
  private:
    std::vector<uint32_t> _colors;
    int _speed;
    int _distance; // distance between lights, in pixels
    ulong _startingFrame = 0;
    double _currentOffset = 0;
};
