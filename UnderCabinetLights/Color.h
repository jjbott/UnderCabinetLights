#include "neopixel.h"
#include "math.h"
#pragma once

class Color
{
  public:
    static uint32_t HsvToColor(int16_t h /* 0 - 359 */, uint8_t s /* 0 - 255 */, uint8_t v /* 0 - 255 */)
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

    static void ColorToHsv(uint32_t rgbColor, int16_t &h, uint8_t &s, uint8_t &v)
    {
      uint8_t r = rgbColor >> 16;
      uint8_t g = rgbColor >> 8;
      uint8_t b = rgbColor;

        double      min, max, delta;

        min = r < g ? r : g;
        min = min < b ? min : b;

        max = r > g ? r : g;
        max = max > b ? max : b;

        v = max;
        delta = max - min;
        if (delta < 1)
        {
            s = 0;
            h = 0; // as good a guess as any
            return;
        }

        if( max > 0 )
        {
          s = 255 * (delta / max);
        }
        else
        {
          // Pretty sure this is already covered by the  "delta < 1" case above
          s = 0;
          h = 0; // A good a guess as any
        }

        double tempHue;
        if( r == max )
        {
            tempHue = ( g - b ) / delta;        // between yellow & magenta
        }
        else if ( g == max )
        {
            tempHue = 2.0 + ( b - r ) / delta;  // between cyan & yellow
        }
        else
        {
            tempHue = 4.0 + ( r - g ) / delta;  // between magenta & cyan
        }

        h = tempHue * 60.0;  // Convert to degrees

        if( h < 0 )
        {
            h += 360;
        }
    }

    static uint32_t Dim(uint32_t rgbColor, double dimAmount)
    {
      if ( dimAmount > 1 ) dimAmount = 1;
      if ( dimAmount < 0 ) dimAmount = 0;

      dimAmount = pow(dimAmount, 1.5);

      uint8_t r = rgbColor >> 16;
      uint8_t g = rgbColor >> 8;
      uint8_t b = rgbColor;

      r *= dimAmount;
      g *= dimAmount;
      b *= dimAmount;

      return Adafruit_NeoPixel::Color(r, g, b);
    }

};
