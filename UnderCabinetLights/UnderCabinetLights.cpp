#include "application.h"
#include "stdarg.h"
#include <vector>
#include <map>
#include <memory>
#include <sstream>

PRODUCT_ID(PLATFORM_ID);
PRODUCT_VERSION(2);

#include "errno.h"

#include "neopixel.h"
#include "Config.h"
#include "Rainbow.h"
#include "StaticColor.h"
#include "Sparkle.h"
#include "Clock.h"
#include "PixelBuffer.h"
#include "Color.h"

/* ======================= prototypes =============================== */

ulong lastPublish = 0;

const ulong MICROS_PER_FRAME = 1000000 / Config::FPS;
ulong frame = 0;

ulong lastFrameTime = 0;

void nextFrame(int elapsedFrames);

/* ======================= extra-examples.cpp ======================== */

STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));

SYSTEM_MODE(AUTOMATIC);


PixelBuffer pixelBuffer;
std::vector<std::unique_ptr<Animation>> animations;

uint32_t getColor(int index);
uint32_t rainbowCycle(int i);
uint32_t stripCycleTest(int i);
void rainbowCycle();

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void debug(String format, ...) {
    char msg [50];

    va_list argptr;
    va_start(argptr, format);
    vsnprintf(msg, 50, format.c_str(), argptr);
    va_end(argptr);

    //sprintf(msg, message.c_str(), value);
    Particle.publish("DEBUG", msg);
}

bool stringToInt(const char* str, int base, int& result)
{
	result = 0;
	char* end;
	long r = strtol(str, &end, base);
	if ( end > str && (uint)(end-str) == strlen(str) && errno != ERANGE && r <= INT_MAX && r >= INT_MIN)
	{
		result = r;
		return true;
	}

	return false;
}

bool stringToInt(const char* str, int& result)
{
  return stringToInt(str, 10, result);
}

uint32_t hsvToColor(uint16_t h /* 0 - 359 */, uint8_t s /* 0 - 255 */, uint8_t v /* 0 - 255 */)
{
    double      hh, p, q, t, ff;
    long        i;

    if(s <= 0.0) {
      return Adafruit_NeoPixel::Color(v, v, v);
    }
    hh = h;
    while ( hh < 0 ) { hh += 360; }
    while ( hh >= 360 ) { hh -= 360; }
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    double vPercent = v/255.0;
    double sPercent = s/255.0;
    p = 255 * vPercent * (1.0 - sPercent);
    q = 255 * vPercent * (1.0 - (sPercent * ff));
    t = 255 * vPercent * (1.0 - (sPercent * (1.0 - ff)));

    switch(i) {
      case 0:
        return Adafruit_NeoPixel::Color(v, t, p);
      case 1:
        return Adafruit_NeoPixel::Color(q, v, p);
      case 2:
        return Adafruit_NeoPixel::Color(p, v, t);
      case 3:
        return Adafruit_NeoPixel::Color(p, q, v);
      case 4:
        return Adafruit_NeoPixel::Color(t, p, v);
      case 5:
      default:
        return Adafruit_NeoPixel::Color(v, p, q);
    }
}

int lightLevel;
int litSegment = -1;

void publishModes()
{
  std::ostringstream ss;
  ss << "m[";

  bool first = true;
  for (auto &anim : animations) {
      if ( !first )
      {
        ss << ",";
      }
      first = false;

      // If this is left as a String object, quotes are added around it.
      // I should probably know why, but I dont. Luckily I want quotes.
      ss << anim->GetDescription();
      //ss << anim->GetDescription().c_str();
  }
  ss << "]";

  std::string str = ss.str();
  UDP Udp;
  Udp.begin(123);
  Udp.sendPacket(str.c_str(), str.length(), Config::PublishToIp, Config::PublishToPort);
}

std::vector<std::string> splitString(std::string str, String delimiter)
{
  std::vector<std::string> result;
  size_t pos = 0;
  std::string token;
  while ((pos = str.find(delimiter)) != std::string::npos) {
      token = str.substr(0, pos);
      result.push_back(token);
      str.erase(0, pos + delimiter.length());
  }
  result.push_back(str);
  return result;
}

std::map<std::string, std::string> parseModeString(std::string modeString)
{
  std::map<std::string, std::string> result;
  // modeString will be formatted like a query string,
  // except with ':' instead of '&' due to cloud function limitations
  std::vector<std::string> keyValues = splitString(modeString, ":");

  for (const std::string& s : keyValues)
  {
    //debug("keyvalue: %s", s.c_str());
    std::vector<std::string> keyValue = splitString(s, "=");
    if ( keyValue.size() != 2)
    {
      // bad data somewhere, wipe everything
      result.clear();
      return result;
    }

    result[keyValue[0]] = keyValue[1];
  }

  return result;
}

bool parseInt(std::map<std::string,std::string> keyValues, std::string key, int &value)
{
  value = 0;
  if ( keyValues.count(key) != 1 )
  {
    debug("Missing key %s or bad count %d", key.c_str(), keyValues.count(key));
    return false;
  }

  if ( !stringToInt(keyValues[key].c_str(), value) )
  {
    debug("Cant parse key from string %s", key.c_str(), keyValues[key].c_str());
    return false;
  }
  return true;
}

bool validate(bool test, String message)
{
  if(!test)
  {
    debug(message);
  }
  return test;
}

int setMode(String mode)
{
  std::map<std::string,std::string> keyValues = parseModeString(std::string(mode));

  const std::string MODE_KEY = "mode";
  const std::string START_KEY = "start";
  const std::string END_KEY = "end";
  const std::string DURATION_KEY = "dur";
  const std::string WIDTH_KEY = "w";
  const std::string COLOR_KEY = "color";
  const std::string THRESHOLD_KEY = "thr";
  const std::string PERCENT_KEY = "percent";
  const std::string RESPECT_LIGHT_LEVEL_KEY = "rll";

  debug("New mode string: %s", mode.c_str());

  int start;
  int end;
  bool respectLightLevel = true;
  if ( keyValues.count(RESPECT_LIGHT_LEVEL_KEY) > 0 && keyValues[RESPECT_LIGHT_LEVEL_KEY] == "0")
  {
    respectLightLevel = false;
  }

  if ( validate(keyValues.size() > 0, "No keyvalues")
    && validate(keyValues.count(MODE_KEY) > 0, "No mode key")
    && parseInt(keyValues, START_KEY, start)
    && parseInt(keyValues, END_KEY, end)
    && validate(start >= 0, "Start is too low")
    && validate(end < PIXEL_COUNT, "End is too high")
    && validate(start <= end, "End is less than start"))
  {
    debug("hmmm");
      auto newMode = keyValues[MODE_KEY];
      if ( newMode == "rainbow")
      {
        int duration;
        int width;
        if ( keyValues.count(DURATION_KEY) > 0
          && keyValues.count(WIDTH_KEY) > 0
          && stringToInt(keyValues[DURATION_KEY].c_str(), duration)
          && stringToInt(keyValues[WIDTH_KEY].c_str(), width))
        {
          animations.push_back(std::unique_ptr<Animation>(new Rainbow(frame, start, end, duration, width, respectLightLevel)));
          return 0;
        }
      }
      else if ( newMode == "staticcolor")
      {
        int color;
        if ( keyValues.count(COLOR_KEY) > 0
          && stringToInt(keyValues[COLOR_KEY].c_str(), 16, color))
        {
          animations.push_back(std::unique_ptr<Animation>(new StaticColor(color, start, end, respectLightLevel)));
          return 0;
        }
      }
      else if ( newMode == "sparkle")
      {
        int color;
        int duration;
        int threshold;
        if ( keyValues.count(COLOR_KEY) > 0
          && parseInt(keyValues, DURATION_KEY, duration)
          && parseInt(keyValues, THRESHOLD_KEY, threshold)
          && stringToInt(keyValues[COLOR_KEY].c_str(), 16, color))
        {
          debug("wha");
          animations.push_back(std::unique_ptr<Animation>(new Sparkle(color, threshold, 1, duration, start, end, respectLightLevel)));
          debug("New Sparkle Added");
          return 0;
        }
      }
  }

  return -1;
}

void setup() {
  pinMode(A0,INPUT);
  pinMode(A5,OUTPUT);
  digitalWrite(A5,HIGH);

  Particle.variable("lightLevel", &lightLevel, INT);
  Particle.variable("rLightLevel", &rawLightLevel, INT);
  Particle.function("setMode", setMode);

  animations.push_back(std::unique_ptr<Animation>(new StaticColor(Adafruit_NeoPixel::Color(255,255,255), 0, 268, true)));
  //animations.push_back(std::unique_ptr<Animation>(new Pluck(Adafruit_NeoPixel::Color(0,0,255), 30, 400, 0, 268, true)));

  publishModes();
}
void loop() {
  //dirty = false;
  loopCounter++;

  if ( Clock::TriggerEveryXMicros(MICROS_PER_FRAME, lastFrameTime) )
  {
      nextFrame();
      //strip1.show();
      //strip2.show();
  }
  /*
    switch((Mode)mode)
    {
        case Mode::Rainbow:
            rainbowCycle();
            break;
        case Mode::StripCycleTest:
            stripCycleTest();
            break;
        default: // off
            break;
    }
    */


  nextFrame(Clock::TriggerEveryXMicros(MICROS_PER_FRAME, lastFrameTime));

  if ( Clock::TriggerEveryXMillis(1000, lastPublish) > 0)
  {
    publishLightLevel(rawLightLevel, lightLevel);
    publishModes();
    publishFps();
  }
}

void nextFrame(int elapsedFrames)
{
  frame += elapsedFrames;

  if ( elapsedFrames == 0 )
  {
    return;
  }

  ++actualFrameCount;

  bool modeChanged = false;
  for(int i = animations.size() - 1; i >= 0; --i)
  {
    animations[i]->Render(frame, lightLevel, pixelBuffer);

    // Remove modes that didnt render anything
    if ( animations[i]->IsObsolete() )
    {
      animations.erase(animations.begin() + i);
      modeChanged = true;
    }
  }
  pixelBuffer.Show();

  if ( modeChanged )
  {
    publishModes();
  }
}
