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

//void colorAll(uint32_t c);
//void rainbowCycle(Adafruit_NeoPixel &strip, uint8_t wait);
//uint32_t Wheel(byte WheelPos);

ulong statusUpdateTriggerMillis = 0;
ulong lastLoopCounterPublish = 0;
ulong loopCounter = 0;


const ulong MICROS_PER_FRAME = 1000000 / Config::FPS;
ulong frame = 0;

ulong lastFrameTime = 0;

void nextFrame();

/* ======================= extra-examples.cpp ======================== */

STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));

SYSTEM_MODE(AUTOMATIC);


PixelBuffer pixelBuffer;
std::vector<std::unique_ptr<Animation>> animations;

uint32_t getColor(int index);
uint32_t rainbowCycle(int i);
uint32_t stripCycleTest(int i);
void rainbowCycle();
/*
bool triggerEveryXMillis(ulong x, unsigned long &lastTriggeredMillis);
bool triggerEveryXMicros(ulong x, unsigned long &lastTriggeredMicros);
*/


//bool dirty = false;


// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

enum Mode
{
    Off = 0,
    Normal = 1,
    Rainbow1 = 2,
    StripCycleTest = 3
};


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

int mode = 0;
int lightLevel;
int litSegment = -1;
/*
void initMode()
{
  switch((Mode)mode)
  {
    case Mode::StripCycleTest:
      litSegment = -1;
      break;
      default: // off
          colorAll(strip1.Color(0,0,0));
          strip1.show();
          strip2.show();
          publishLightStatus();
          break;
  }
}
*/

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

      //ss << "\"" << anim->GetDescription() << "\"";
      ss << anim->GetDescription();
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

/*
    int newMode;
    if ( stringToInt(mode.c_str(), newMode) )
    {
      ::mode = newMode;
      //initMode();

      char buffer[2];
      buffer[0] = 'm';
      buffer[1] = (char)::mode;
      UDP Udp;
      Udp.begin(123);
      Udp.sendPacket(buffer, 2, Config::PublishToIp, Config::PublishToPort);


      return 0;
    }
    return -1;
    */
}

void setup() {
    pinMode(A0,INPUT);
    pinMode(A5,OUTPUT);
    digitalWrite(A5,HIGH);

    Particle.variable("lightLevel", &lightLevel, INT);
    Particle.variable("mode", &mode, INT);
    Particle.function("setMode", setMode);

    animations.push_back(std::unique_ptr<Animation>(new StaticColor(Adafruit_NeoPixel::Color(255,255,255), 0, 268, true)));
/*
  animations.push_back(std::unique_ptr<Animation>(new Rainbow(frame, 0, 50, 500, 10, true)));
  animations.push_back(std::unique_ptr<Animation>(new Rainbow(frame, 50, 150, 1500, 30, true)));
  animations.push_back(std::unique_ptr<Animation>(new StaticColor(255, 150, 200, true)));
  animations.push_back(std::unique_ptr<Animation>(new Sparkle(Adafruit_NeoPixel::Color(255,0,0) , 20, 1, 1000, 200, PIXEL_COUNT - 1, true)));
  animations.push_back(std::unique_ptr<Animation>(new Sparkle(Adafruit_NeoPixel::Color(0,255,0) , 40, 1, 2000, 25, 75, true)));
*/
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


    //publishLightStatus();

/*
    if ( Clock::TriggerEveryXMillis(1000, lastLoopCounterPublish))
    {
      debug("Loop counter: %lu, frame: %lu", loopCounter, frame);
      loopCounter = 0;
    }
*/
}

void nextFrame()
{
  ++frame;

  lightLevel = analogRead(A0);

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

  if ( Clock::TriggerEveryXMillis(1000, lastLoopCounterPublish))
  {
    publishModes();
  }
//  for(int i=0; i< PIXEL_COUNT; i++) {
//    setColor(i, rainbowCycle(i));

/*
    if ( i/10 % 2 == 0)
    {
      setColor(i, rainbowCycle(i));
    }
    else
    {
      setColor(i, sparkle(i));
    }
*/
//  }
}

// Set all pixels in the strip to a solid color, then wait (ms)
/*
void colorAll(uint32_t c) {
  uint16_t i;
  for(i=0; i<(strip1.numPixels() + strip2.numPixels()); i++) {
    setColor(i, c);
  }
}
*/

/*
// Fill the dots one after the other with a color, wait (ms) after each one
void colorWipe(Adafruit_NeoPixel &strip, uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(Adafruit_NeoPixel &strip, uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(strip, (i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
*/

/*

bool segmentToIndex(int segment, int& index, int& count)
{
  const int segmentSizes[] = {53, 86, 29, 28, 30, 43};
  index = 0;
  count = 0;

  if ( segment >= 0 && segment < 6 )
  {
    for(int i = 0; i <= segment; ++i)
    {
      index += count;
      count = segmentSizes[i];
    }
    return true;
  }
  return false;
}

const uint32_t stripCycleDurationMs = 10000;
const uint32_t stripCycleDurationFrames = stripCycleDurationMs * Config::FPS / 1000;

int stripCycleTestCycle()
{
  return (frame % stripCycleDurationFrames) * 10 / stripCycleDurationFrames;
}

uint32_t stripCycleTest(int i)
{
  int segment = stripCycleTestCycle();

  if ( segment >= 0 && segment < 6 )
  {
    int startIndex = 0;
    int count = 0;
    segmentToIndex(segment, startIndex, count);
    if ( i >= startIndex && i < (startIndex + count))
    {
      return strip1.Color(255,255,255);
    }
    else
    {
      return strip1.Color(0,0,0);
    }
  }
  else if ( segment >=6 )
  {
    return strip1.Color(255,255,255);
  }

  return 0; // Cant ever get here, but compiler gives warning otherwise
}

void stripCycleTest()
{
    colorAll(strip1.Color(0,0,0));

    litSegment++;
    if ( litSegment >= 7 ) {
        litSegment = 0;
    }

    int startIndex = 0;
    int count = 0;
    int delayMs = 1000;

    if ( litSegment >= 0 && litSegment < 6 )
    {
      segmentToIndex(litSegment, startIndex, count);
    }
    else if ( litSegment >=6 )
    {
      startIndex = 0;
      count = strip1.numPixels() + strip2.numPixels();
      delayMs = 10000;
    }

    for(int i = 0; i < count; ++i)
    {
        setColor(startIndex + i, strip1.Color(255,255,255));
    }

    strip1.show();
    strip2.show();
    publishLightStatus();
    delay(delayMs);
}

const uint32_t cycleDurationMs = 2000;
const uint32_t cycleDurationFrames = cycleDurationMs * Config::FPS / 1000;

uint16_t rainbowCycleStep()
{
  return (frame % cycleDurationFrames) * 360 / cycleDurationFrames;
}

uint32_t reverseRainbowCycle(int i)
{
  int pixelCount = strip1.numPixels() + strip2.numPixels();
  return hsvToColor(360-(i * 360 / pixelCount) + rainbowCycleStep(), 255,255);
}

uint32_t rainbowCycle(int i) {
  int pixelCount = strip1.numPixels() + strip2.numPixels();

  return hsvToColor((i * 360 / pixelCount) + rainbowCycleStep(), 255,255);
  //return Wheel(((i * 256 / pixelCount) + rainbowCycleStep() ));
}

void rainbowCycle() {
  int pixelCount = strip1.numPixels() + strip2.numPixels();
  uint16_t i;
    for(i=0; i< pixelCount; i++) {
      setColor(i, rainbowCycle(i));
    }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return Adafruit_NeoPixel::Color(255 - WheelPos * 3, WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return Adafruit_NeoPixel::Color(0, 255 - WheelPos * 3, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return Adafruit_NeoPixel::Color(WheelPos * 3, 0, 255 - WheelPos * 3);
  }
}

uint32_t sparkleDecayDurationMs = 1000;
uint32_t sparkleDurationFrames = sparkleDecayDurationMs * Config::FPS / 1000;
uint32_t sparkleDecayPerFrame = 256 / sparkleDurationFrames;
uint8_t sparkleMinHue = 0;
uint8_t sparkleMaxHue = 80;
float sparkleCountPer10000 = 100;

uint32_t sparkle(int i)
{
  if ( rand() % 10000 < sparkleCountPer10000 )
  {
    // new sparkle!!1
    uint8_t hue = rand() % (sparkleMaxHue - sparkleMinHue) + sparkleMinHue;
    return Wheel(hue);
  }
  else
  {
      uint32_t color = getColor(i);
      int r = -sparkleDecayPerFrame + (byte)(color>>16);
      int g = -sparkleDecayPerFrame + (byte)(color>>8);
      int b = -sparkleDecayPerFrame + (byte)(color);
      if ( r < 0) r = 0;
      if ( g < 0) g = 0;
      if ( b < 0) b = 0;
      color = (r << 16) + (g << 8) + 0;//(b);
      return color;
  }
}
*/
