#include "neopixel.h"

/* ======================= prototypes =============================== */

void colorAll(Adafruit_NeoPixel &strip, uint32_t c, uint8_t wait);
void rainbowCycle(Adafruit_NeoPixel &strip, uint8_t wait);
uint32_t Wheel(byte WheelPos);

/* ======================= extra-examples.cpp ======================== */

STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));

SYSTEM_MODE(AUTOMATIC);

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 43
#define PIXEL_PIN D6
#define PIXEL_TYPE WS2812B2

#define PIXEL_COUNT_STRIP2 226
#define PIXEL_PIN_STRIP2 D5

// Parameter 1 = number of pixels in strip
//               note: for some stripes like those with the TM1829, you
//                     need to count the number of segments, i.e. the
//                     number of controllers in your stripe, not the number
//                     of individual LEDs!
// Parameter 2 = pin number (most are valid)
//               note: if not specified, D2 is selected for you.
// Parameter 3 = pixel type [ WS2812, WS2812B, WS2812B2, WS2811,
//                             TM1803, TM1829, SK6812RGBW ]
//               note: if not specified, WS2812B is selected for you.
//               note: RGB order is automatically applied to WS2811,
//                     WS2812/WS2812B/WS2812B2/TM1803 is GRB order.
//
// 800 KHz bitstream 800 KHz bitstream (most NeoPixel products
//               WS2812 (6-pin part)/WS2812B (4-pin part)/SK6812RGBW (RGB+W) )
//
// 400 KHz bitstream (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//                   (Radio Shack Tri-Color LED Strip - TM1803 driver
//                    NOTE: RS Tri-Color LED's are grouped in sets of 3)

// LEDs per segment
// 53
// 86
// 29
// 28
// 30
// 43

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(PIXEL_COUNT_STRIP2, PIXEL_PIN_STRIP2, PIXEL_TYPE);

uint32_t getColor(int index);
uint32_t rainbowCycle(int i);
uint32_t stripCycleTest(int i);
void rainbowCycle();

bool triggerEveryXMillis(ulong x, unsigned long &lastTriggeredMillis);
bool triggerEveryXMicros(ulong x, unsigned long &lastTriggeredMicros);


bool dirty = false;
IPAddress remoteIP(192, 168, 1, 20);
int port = 4040;

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

enum Mode
{
    Off = 0,
    Normal = 1,
    Rainbow = 2,
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

bool stringToInt(const char* str, int& result)
{
	result = 0;
	char* end;
	long r = strtol(str, &end, 10);
	if ( end > str && (end-str) == strlen(str) && errno != ERANGE && r <= INT_MAX && r >= INT_MIN)
	{
		result = r;
		return true;
	}

	return false;
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
int analogvalue;
int litSegment = -1;

bool dirtyPublished = false;
ulong lastPublish = 0;
void publishLightStatus()
{
  if ( !dirtyPublished || dirty)
  {
    dirtyPublished = false;
    if ( triggerEveryXMillis(100, lastPublish) )
    {
      int count = strip1.numPixels() + strip2.numPixels();
      //uint32_t colors[count];

      byte buffer[1 + (count * 3)];
      buffer[0] = 's';

      for(int i = 0; i < count; ++i)
      {
        uint32_t color = getColor(i);
        buffer[1+ (i*3)] = (char)(color >> 16);
        buffer[1+ (i*3)+1] = (char)(color >> 8);
        buffer[1+ (i*3)+2] = (char)color;
      }

      UDP Udp;
      Udp.begin(123);
      Udp.sendPacket(buffer, sizeof(buffer), remoteIP, port);
      dirtyPublished = true;
    }
  }
}

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

int setMode(String mode)
{
    int newMode;
    if ( stringToInt(mode.c_str(), newMode) )
    {
      ::mode = newMode;
      initMode();

      char buffer[2];
      buffer[0] = 'm';
      buffer[1] = (char)::mode;
      UDP Udp;
      Udp.begin(123);
      Udp.sendPacket(buffer, 2, remoteIP, port);


      return 0;
    }
    return -1;
}

void setup() {
    pinMode(A0,INPUT);
    pinMode(A5,OUTPUT);
    digitalWrite(A5,HIGH);

    Particle.variable("analogvalue", &analogvalue, INT);
    Particle.variable("mode", &mode, INT);
    Particle.function("setMode", setMode);
  strip1.begin();
  strip2.begin();

  debug("Version: %08x", System.versionNumber());
  colorAll(strip1.Color(0,0,0));
  colorAll(strip2.Color(0,0,0));

  strip1.show();
  strip2.show();

}

unsigned long elapsedMillis(unsigned long now, unsigned long since)
{
  unsigned long elapsed = now - since;
  if ( now < since )
  {
    // time overflowed, fixed elapsed time
    elapsed = (ULONG_MAX - since) + now;
  }

  return elapsed;
}

unsigned long elapsedMicros(unsigned long now, unsigned long since)
{
  const ulong MAX_MICROS = ULONG_MAX; //35791394;
  unsigned long elapsed = now - since;
  if ( now < since )
  {
    // time overflowed, fixed elapsed time
    elapsed = (MAX_MICROS - since) + now;
  }

  return elapsed;
}

ulong lastMillis = 0;
ulong lastMicros = 0;
ulong lastCycleCount = 0;

bool triggerEveryXMillis(ulong x, unsigned long &lastTriggeredMillis)
{
  return triggerEveryXMicros(x * 1000, lastTriggeredMillis);

  //ulong now = millis();
  //ulong elapsed = elapsedMillis(now, lastTriggeredMillis);
  ulong now = DWT->CYCCNT / SYSTEM_US_TICKS; // micros();
  ulong elapsed = elapsedMicros(now, lastTriggeredMillis) / 1000;
  if ( elapsed > x )
  {
    /*
    ulong startMillis = millis();
    ulong startMicros = micros();
    ulong startCycleCount = DWT->CYCCNT;
    debug("%s, %lu, %lu, %lu", Time.timeStr().c_str(), startMillis - lastMillis, startMicros - lastMicros, (startCycleCount - lastCycleCount)/SYSTEM_US_TICKS);
    lastMillis = startMillis;
    lastMicros = startMicros;
    lastCycleCount = startCycleCount;
*/
    //debug("%s %lu %lu", Time.timeStr().c_str(),millis(),micros());
    //debug("now: %lu, then %lu, rawdiff %lu, diff %lu, x %l", now, lastTriggeredMillis, elapsedMicros(now, lastTriggeredMillis), elapsed, x);
    lastTriggeredMillis = now;
    return true;
  }
  return false;
}

bool triggerEveryXMicros(ulong x, ulong &lastTriggeredMicros)
{
  ulong now = DWT->CYCCNT / SYSTEM_US_TICKS;
  ulong elapsed = elapsedMicros(now, lastTriggeredMicros);
  if ( elapsed > x )
  {
    /*
    if ( elapsed %100 == 0)
    {
      debug("%lu, %lu, %lu, %lu", x, now - lastTriggeredMicros, now, lastTriggeredMicros);
    }
    */
    lastTriggeredMicros = now;
    return true;
  }
  return false;
}

uint32_t getColor(int index)
{
    if ( index < strip2.numPixels() )
    {
        return strip2.getPixelColor(strip2.numPixels() - index - 1);
    }
    else
    {
        return strip1.getPixelColor(index - strip2.numPixels());
    }
}

void setColor(int index, uint32_t color)
{
  dirty = true;
    if ( index < strip2.numPixels() )
    {
        strip2.setPixelColor(strip2.numPixels() - index - 1, color);
    }
    else
    {
        strip1.setPixelColor(index - strip2.numPixels(), color);
    }
}



ulong statusUpdateTriggerMillis = 0;
ulong lastLoopCounterPublish = 0;
ulong loopCounter = 0;

const uint8_t FPS = 60;
const ulong MICROS_PER_FRAME = 1000000 / FPS;
ulong frame = 0;

ulong lastFrameTime = 0;

void nextFrame();

void loop() {
  dirty = false;
  loopCounter++;

  if ( triggerEveryXMicros(MICROS_PER_FRAME, lastFrameTime) )
  {
      nextFrame();
      strip1.show();
      strip2.show();
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


    publishLightStatus();


    if ( triggerEveryXMillis(1000, lastLoopCounterPublish))
    {
      debug("Loop counter: %lu, frame: %lu", loopCounter, frame);
      loopCounter = 0;
    }

}

void nextFrame()
{
  ++frame;
  for(int i=0; i<(strip1.numPixels() + strip2.numPixels()); i++) {
    setColor(i, reverseRainbowCycle(i));
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
  }
}

// Set all pixels in the strip to a solid color, then wait (ms)
void colorAll(uint32_t c) {
  uint16_t i;
  for(i=0; i<(strip1.numPixels() + strip2.numPixels()); i++) {
    setColor(i, c);
  }
}

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
const uint32_t stripCycleDurationFrames = stripCycleDurationMs * FPS / 1000;

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
const uint32_t cycleDurationFrames = cycleDurationMs * FPS / 1000;

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
  /*if ( i == 0 )
  {
    debug("%lu %s %s", frame, String(rainbowCycleStep()).c_str(), String((i * 360 / pixelCount) + rainbowCycleStep()).c_str());
  }*/
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
uint32_t sparkleDurationFrames = sparkleDecayDurationMs * FPS / 1000;
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
