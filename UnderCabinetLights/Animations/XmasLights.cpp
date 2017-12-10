#include "XmasLights.h"
#include "Color.h"
#include "Config.h"
#include "math.h"

XmasLights::XmasLights(
  std::vector<uint32_t> colors,
  int start,
  int end,
  int speed,
  int distance, // distance between lights, in pixels)
  String friendlyDescription)
  : Animation(start, end, friendlyDescription),
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
  int di = i - (int)_currentOffset;

  // Make sure di is position. mod on negative numbers is haunted.
  di += (_distance * _colors.size());

  int mod = di%_distance;
  if ( mod == 0)
  {
    return _colors[(di/_distance) % _colors.size()];
    //return  Color::Dim(_colors[(i/_distance) % _colors.size()], 1.0 - fraction);
  }
  else if (mod == 1)
  {
    return  Color::Dim(_colors[((di-1)/_distance) % _colors.size()], fraction);
  }

  return 0;
}

String XmasLights::GetDescription()
{
  if ( _friendlyDescription != "" ) return _friendlyDescription;

  return String::format("XmasLights: Color #%06X, Start %d, End %d", _colors[0], (int)_start, (int)_end);
}
