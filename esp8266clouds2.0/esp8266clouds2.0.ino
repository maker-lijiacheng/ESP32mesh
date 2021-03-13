
#include <SoftwareSerial.h>
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <Adafruit_NeoPixel.h>
#define PIN        12 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 600 // Popular NeoPixel ring size
#define MICROWAVEPIN 16 //微波传感器管脚
SoftwareSerial mySerial(13, 14);//MP3模块
DFRobotDFPlayerMini myPlayer;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// 静态函数 WS2812
uint32_t rgbToColor(uint8_t r, uint8_t g, uint8_t b)
{
  return (uint32_t)((((uint32_t)r<<16) | ((uint32_t)g<<8)) | (uint32_t)b);
}

void setup() {
    #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
    #endif
  // put your setup code here, to run once:
    Serial.begin(9600);
    mySerial.begin(9600);
    myPlayer.begin(mySerial);
    myPlayer.play(1);

    strip.begin();//WS2812初始化
    strip.clear();
    strip.show();
}

void loop() {
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
      strip.setPixelColor(i, strip.Color(200, 150, 0));
      strip.show(); 
      delay(20);
  }
  strip.clear();
  strip.show();
  myPlayer.play(1);
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
      strip.setPixelColor(i, strip.Color(10, 150, 255));
      strip.show(); 
      delay(20);
  }
  strip.clear();
  strip.show();
  myPlayer.play(2);
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
      strip.setPixelColor(i, strip.Color(0, 150, 200));
      strip.show(); 
      delay(20);
  }
  strip.clear();
  strip.show();
  myPlayer.play(3);
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
      strip.setPixelColor(i, strip.Color(200, 0, 100));
      strip.show(); 
      delay(20);
  }
  strip.clear();
  strip.show();
  myPlayer.play(4);
}
