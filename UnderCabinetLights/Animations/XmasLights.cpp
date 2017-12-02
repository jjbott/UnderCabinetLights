#include "XmasLights.h"
#include "Color.h"
#include "Config.h"
#include "math.h"

XmasLights::XmasLights(
  std::vector<uint32_t> colors,
  int start,
  int end,
  int speed,
  int distance) // distance between lights, in pixels)
  : Animation(start, end, false),
  _colors{std::move(colors)},
  _distance{distance},
  _speed{speed}
{
}

void XmasLights::UpdateFrame(ulong frame)
{
  Animation::UpdateFrame(frame);
  
  double cycleDurationFrames = _speed * Config::FPS / 1000.0;
  _currentOffset = fmod((frame - _startingFrame), cycleDurationFrames) * _distance * _colors.size() / cycleDurationFrames;
}

uint32_t XmasLights::GenerateColor(int i, const PixelBuffer &pb)
{
  double fraction = _currentOffset - (int)_currentOffset;
  i -= (int)_currentOffset;
  if (i%_distance==0)
  {
    //return _colors[(i/_distance) % _colors.size()];
    return  Color::Dim(_colors[(i/_distance) % _colors.size()], 1.0 - fraction);
  }
  else if ((i-1)%_distance==0)
  {
    return  Color::Dim(_colors[((i-1)/_distance) % _colors.size()], fraction);
  }
  else return 0;
}

String XmasLights::GetDescription()
{
  char buffer[100];
  snprintf(buffer, 100, "XmasLights: Color #%06X, Start %d, End %d", _colors[0], (int)_start, (int)_end);
  return String(buffer);
}
