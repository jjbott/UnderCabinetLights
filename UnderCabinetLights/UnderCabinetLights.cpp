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
#include "TestPattern.h"
#include "Animations/XmasLights.h"
#include "Animations/Decay.h"
#include "Animations/Rotator.h"

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
std::vector<std::shared_ptr<Animation>> animations;
ulong manualAnimationStartTime;
std::shared_ptr<Animation> manualAnimation;

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

void publishModes()
{
  std::ostringstream ss;
  ss << "m[";

  if ( manualAnimation != nullptr )
  {
    ss << manualAnimation->GetDescription();
  }
  else
  {
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
  }
  ss << "]";

  std::string str = ss.str();
  UDP Udp;
  Udp.begin(123);
  Udp.sendPacket(str.c_str(), str.length(), Config::PublishToIp, Config::PublishToPort);
}

int actualFrameCount = 0;
void publishFps()
{
  std::ostringstream ss;
  ss << "fps";
  ss << actualFrameCount;
  ss << ",";
  ss << (int)Config::FPS;

  actualFrameCount = 0;

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

std::shared_ptr<Animation> CreateModeAnimation(String mode)
{
  if ( mode == "WhiteSparkle" )
  {
    return std::shared_ptr<Animation>(new Sparkle({Adafruit_NeoPixel::Color(255,255,255)}, 10, .5, 1000, 0, PIXEL_COUNT, false));
  }
  else if ( mode == "WhiteMarquee" )
  {
    return std::shared_ptr<Animation>(
      new Decay(
        new XmasLights(
          {
            Adafruit_NeoPixel::Color(255,255,255)
          }, 0, PIXEL_COUNT, 1000, 10
      ), .7));
  }
  else if ( mode == "WhiteMarquee_Reverse" )
  {
    return std::shared_ptr<Animation>(
      new Decay(
        new XmasLights(
          {
            Adafruit_NeoPixel::Color(255,255,255)
          }, 0, PIXEL_COUNT, -1000, 10
      ), .7));
  }
  else if ( mode == "RainbowSparkle" )
  {
    return std::shared_ptr<Animation>(new Sparkle({
      Adafruit_NeoPixel::Color(255,0,0), // R
      Adafruit_NeoPixel::Color(0,0,255), // B
      Adafruit_NeoPixel::Color(255,128,0), // Y
      Adafruit_NeoPixel::Color(178,0,255), // V
      Adafruit_NeoPixel::Color(255,55,0), // O
      Adafruit_NeoPixel::Color(0,255,0) // G
    }, 10, .4, 2000, 0, PIXEL_COUNT, false));
  }
  else if ( mode == "RainbowMarquee" )
  {
    return std::shared_ptr<Animation>(
      new Decay(
        new XmasLights(
          {
            Adafruit_NeoPixel::Color(255,0,0), // R
            Adafruit_NeoPixel::Color(0,0,255), // B
            Adafruit_NeoPixel::Color(255,128,0), // Y
            Adafruit_NeoPixel::Color(178,0,255), // V
            Adafruit_NeoPixel::Color(255,55,0), // O
            Adafruit_NeoPixel::Color(0,255,0) // G
          }, 0, PIXEL_COUNT, 6000, 10)
      , .9));
  }
  else if ( mode == "RainbowMarquee_Reverse" )
  {
    return std::shared_ptr<Animation>(
      new Decay(
        new XmasLights(
          {
            Adafruit_NeoPixel::Color(255,0,0), // R
            Adafruit_NeoPixel::Color(0,0,255), // B
            Adafruit_NeoPixel::Color(255,128,0), // Y
            Adafruit_NeoPixel::Color(178,0,255), // V
            Adafruit_NeoPixel::Color(255,55,0), // O
            Adafruit_NeoPixel::Color(0,255,0) // G
          }, 0, PIXEL_COUNT, -6000, 10)
      , .9));
  }
  else if ( mode == "RedGreenSparkle" )
  {
    return std::shared_ptr<Animation>(new Sparkle({Adafruit_NeoPixel::Color(255,0,0), Adafruit_NeoPixel::Color(0,255,0)}, 10, .5, 1000, 0, PIXEL_COUNT, false));
  }
  else if ( mode == "RedGreenMarquee" )
  {
    return std::shared_ptr<Animation>(
        new XmasLights(
          {
            Adafruit_NeoPixel::Color(255,0,0), // R
            Adafruit_NeoPixel::Color(0,255,0) // G
          }, 0, PIXEL_COUNT, 1000, 5)
        );
  }
  else if ( mode == "RedGreenMarquee_Reverse" )
  {
    return std::shared_ptr<Animation>(
        new XmasLights(
          {
            Adafruit_NeoPixel::Color(255,0,0), // R
            Adafruit_NeoPixel::Color(0,255,0) // G
          }, 0, PIXEL_COUNT, -1000, 5)
        );
  }
  else
  {
    return nullptr;
  }
}

int setMode(String mode)
{
  debug("New mode string: %s %s %s", mode.c_str(), mode.substring(0, 5).c_str(), mode.substring(5).c_str());

  if ((mode.length() <= 5) || (mode.substring(0, 5) != "xmas_"))
  {
    return -1;
  }


  if ((mode.length() > 5) && (mode.substring(0, 5) == "xmas_"))
  {
    String newMode = mode.substring(5);
    manualAnimationStartTime = Time.now();

    manualAnimation = CreateModeAnimation(newMode);
  }

  return 0;
}

void setup() {

  pinMode(A0,INPUT);
  pinMode(A5,OUTPUT);
  digitalWrite(A5,HIGH);

  Particle.function("setMode", setMode);

  // boot loop detection
  uint8_t eepromInitialized;
  uint32_t now = Time.now();

  EEPROM.get(0, eepromInitialized);
  if ( eepromInitialized != 0 )
  {
    EEPROM.put(0, 0);
    EEPROM.put(1, now);
    Particle.publish("DEBUG", "Initialized boot loop detection");
  }
  else
  {
    uint32_t lastBootTime;
    EEPROM.get(1, lastBootTime);
    EEPROM.put(1, now);
    if ( (now - lastBootTime) < 30 )
    {
      // We're probably in a boot loop. Reboot into safe mode so I can fix the firmware
      Particle.publish("DEBUG", "Boot loop detected, entering safe mode");
      System.enterSafeMode();
    }
    else
    {
        Particle.publish("DEBUG", String::format("Boot loop detection passed, everything is fine I hope! %d", (now - lastBootTime)).c_str() );
    }
  }

  //animations.push_back(std::unique_ptr<Animation>(new StaticColor(Adafruit_NeoPixel::Color(255,255,255), 0, 268, true)));
  //animations.push_back(std::unique_ptr<Animation>(new TestPattern(0, PIXEL_COUNT, false)));

  //animations.push_back(std::unique_ptr<Animation>(new XmasLights({Adafruit_NeoPixel::Color(255,255,255)}, 0, PIXEL_COUNT, 0, 1)));


  animations.push_back(
    std::shared_ptr<Animation>(
      new Rotator({
        std::shared_ptr<Animation>(
          new Decay(
            new XmasLights(
              {
                Adafruit_NeoPixel::Color(255,0,0), // R
                Adafruit_NeoPixel::Color(0,0,255), // B
                Adafruit_NeoPixel::Color(255,128,0), // Y
                Adafruit_NeoPixel::Color(178,0,255), // V
                Adafruit_NeoPixel::Color(255,55,0), // O
                Adafruit_NeoPixel::Color(0,255,0) // G
              }, 0, PIXEL_COUNT, 6000, 10)
          , .9))
        ,std::shared_ptr<Animation>(
            new Decay(
              new XmasLights(
                {
                  Adafruit_NeoPixel::Color(255,0,0), // R
                  Adafruit_NeoPixel::Color(0,0,255), // B
                  Adafruit_NeoPixel::Color(255,128,0), // Y
                  Adafruit_NeoPixel::Color(178,0,255), // V
                  Adafruit_NeoPixel::Color(255,55,0), // O
                  Adafruit_NeoPixel::Color(0,255,0) // G
                }, 0, PIXEL_COUNT, 6000, 10)
            , .9))
        ,std::shared_ptr<Animation>(new Sparkle({Adafruit_NeoPixel::Color(255,255,255)}, 10, .5, 1000, 0, PIXEL_COUNT, false))
        ,std::shared_ptr<Animation>(
          new Decay(
            new XmasLights(
              {
                Adafruit_NeoPixel::Color(255,255,255)
              }, 0, PIXEL_COUNT, 1000, 10
          ), .7))
          ,std::shared_ptr<Animation>(
            new Decay(
              new XmasLights(
                {
                  Adafruit_NeoPixel::Color(255,255,255)
                }, 0, PIXEL_COUNT, -1000, 10
            ), .7))
        , std::shared_ptr<Animation>(new Sparkle({Adafruit_NeoPixel::Color(255,0,0), Adafruit_NeoPixel::Color(0,255,0)}, 10, .5, 1000, 0, PIXEL_COUNT, false))
        , std::shared_ptr<Animation>(new Sparkle({
          Adafruit_NeoPixel::Color(255,0,0), // R
          Adafruit_NeoPixel::Color(0,0,255), // B
          Adafruit_NeoPixel::Color(255,128,0), // Y
          Adafruit_NeoPixel::Color(178,0,255), // V
          Adafruit_NeoPixel::Color(255,55,0), // O
          Adafruit_NeoPixel::Color(0,255,0) // G
        }, 10, .4, 2000, 0, PIXEL_COUNT, false))
        ,std::shared_ptr<Animation>(
            new XmasLights(
              {
                Adafruit_NeoPixel::Color(255,0,0), // R
                Adafruit_NeoPixel::Color(0,255,0) // G
              }, 0, PIXEL_COUNT, 1000, 5)
            )
      }, 0, PIXEL_COUNT, 30, 1, true)
    ));

/*
    animations.push_back(std::unique_ptr<Animation>(
      new XmasLights(
        {
          Adafruit_NeoPixel::Color(255,0,0), // R
          Adafruit_NeoPixel::Color(0,255,0) // G
        }, 0, PIXEL_COUNT, 10, 1, 0)));
*/

  //animations.push_back(std::unique_ptr<Animation>(new Pluck(Adafruit_NeoPixel::Color(0,0,255), 30, 400, 0, 268, true)));

  publishModes();
}


void loop() {

  nextFrame(Clock::TriggerEveryXMicros(MICROS_PER_FRAME, lastFrameTime));

  if ( Clock::TriggerEveryXMillis(1000, lastPublish) > 0)
  {
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

  if ( manualAnimation.get() != nullptr && Clock::TriggerInXSeconds(60, manualAnimationStartTime) )
  {
    debug("killing manual animation");
    manualAnimation = nullptr;
  }

  if ( manualAnimation.get() != nullptr )
  {
    manualAnimation->Render(frame, pixelBuffer);
  }
  else
  {
    for(int i = animations.size() - 1; i >= 0; --i)
    {
      animations[i]->Render(frame, pixelBuffer);

      // Remove modes that didnt render anything
      if ( animations[i]->IsObsolete() )
      {
        animations.erase(animations.begin() + i);
        modeChanged = true;
      }
    }
  }

  pixelBuffer.Show();

  if ( modeChanged )
  {
    publishModes();
  }
}
