#include "Secret.h"
#include "Clock.h"
#include "Pluck.h"
#include "neopixel.h" // for Adafruit_NeoPixel::Color
#include <sstream>

Secret::Secret()
{
  _lastKnock = 0;
}

Animation* Secret::Knock(int test, int test2)
{
  uint32_t now = Clock::Micros();
  uint32_t elapsed = Clock::ElapsedMicros(now, _lastKnock);

/*
  std::ostringstream ss;
  ss << elapsed << ", " << test << ", " << test2 << ", " << now << ", " << _lastKnock;
  std::string str = ss.str();
  UDP Udp;
  Udp.begin(123);
  Udp.sendPacket(str.c_str(), str.length(), {192,168,1,20}, 4040);
*/
  _lastKnock = now;

  if ( elapsed > (10 * 1000000) )
  {
    _knocks.clear();
  }
  else
  {
    _knocks.push_back(elapsed);
    if ( _knocks.size() > 7 )
    {
      _knocks.pop_front();
    }

    if ( _knocks.size() == 7 )
    {
      std::ostringstream ss;

      uint32_t avg = 0;
      for (uint32_t &knock : _knocks) {
        avg += knock;
        ss << knock << " ";
      }
      avg /= _knocks.size();

      ss << avg << " ";

      bool unlocked = true;
      for (uint32_t &knock : _knocks) {
        uint32_t normalized = map(knock, 0, avg, 0, 1000);
        ss << normalized << " ";
        if ( normalized > 1050 || normalized < 950 )
        {
          unlocked = false;
        }
      }

      ss << (unlocked?"true":"false") << " ";

      std::string str = ss.str();
      //UDP Udp;
      //Udp.begin(123);
      //Udp.sendPacket(str.c_str(), str.length(), {192,168,1,20}, 4040);

      if ( unlocked )
      {
        // Everything checked out? Holy cow.
        return new Pluck(Adafruit_NeoPixel::Color(0,0,255), 30, avg/1000, 0, 268, false);
      }
    }
  }
  return NULL;
}
