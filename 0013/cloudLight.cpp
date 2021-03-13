#include "Arduino.h"
#include "config.h"

void playCloudLightEffect(byte num)
{
  // if (ISMASTER == 0)
    // slaveCloudRandomDelay();
  switch (num)
  {


    // case 2:
    // 流动的绿色
    // flowingCycle(flowingColorRed, flowingColorGreen, flowingColorBlue, flowingDelayTime);
    // break;

    case 2:
      // 闪电
      lightening(lighteningColorRed, lighteningColorGreen, lighteningColorBlue, lighteningDelayTime, lighteningDelayTime2, lighteningDelayTime3);
      break;
    case 3:
      // 七彩
      rainbowCycle(rainbowDelayTime);
      break;
    default:
      break;
  }
}


// 闪电
void lightening(uint16_t r, uint16_t g, uint16_t b, uint8_t t1, uint8_t t2, uint16_t t3)
{
  strip.clear();
  strip.show();

  uint8_t t = lighteningCycleTimes;
  uint16_t length = strip.numPixels() / 4;

  while (t)
  {

    lighteningSegment(r, g, b, 1, length);
    lighteningSegment(r, g, b, 3, length);
    delay(t1);
    strip.clear();
    strip.show();

    lighteningSegment(r, g, b, 2, length);
    lighteningSegment(r, g, b, 4, length);
    delay(t2);
    strip.clear();
    strip.show();

    lighteningSegment(r, g, b, 1, length);
    lighteningSegment(r, g, b, 3, length);
    delay(t1);
    strip.clear();
    strip.show();

    lighteningSegment(r, g, b, 2, length);
    lighteningSegment(r, g, b, 4, length);
    delay(t2);
    strip.clear();
    strip.show();

    delay(t3);
    t--;
  }
}

void lighteningSegment(uint16_t r, uint16_t g, uint16_t b, byte num, uint16_t length) // num是第几段
{
  for (uint16_t i = length * (num - 1); i < length * num; i++)
  {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }

  strip.show();
}

// 流动
// void flowingCycle(uint16_t r, uint16_t g, uint16_t b, uint8_t wait)
// {

//     uint16_t length = strip.numPixels() / 4;

//     for (uint16_t i = 0; i < strip.numPixels() * flowingCycleTimes; i++)
//     {

//         uint16_t pos = i % strip.numPixels();
//         strip.clear();
//         if (pos < length)
//         {

//             for (uint16_t j = 0; j < pos; j++)
//             {
//                 strip.setPixelColor(j, strip.Color(r, g, b));
//             }
//             for (uint16_t j = 0; j < length - pos; j++)
//             {
//                 strip.setPixelColor(strip.numPixels() - j, strip.Color(r, g, b));
//             }
//         }
//         else
//         {

//             for (uint16_t j = pos - length; j <= pos; j++)
//             {

//                 strip.setPixelColor(j, strip.Color(r, g, b));
//             }
//         }
//         strip.show();
//         delay(wait);
//     }
// }

// 彩虹
void rainbowCycle(uint8_t wait)
{

  for (uint16_t j = 0; j < 256 * rainbowCycleTimes; j++)
  { // 5 cycles of all colors on wheel
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// void slaveCloudRandomDelay()
// {
//   randomSeed(analogRead(RAMDOMPIN));
//   long randomTime = random(0, slaveCloudRandomDelayMax);
//   Serial.println(randomTime);
//   delay(randomTime);
// }
