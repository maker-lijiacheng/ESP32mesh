
#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// ==========配置常量==========
// 引脚
#define STRIPPIN 5 // 灯带输出信号引脚  小云为12 大云为5
// #define RAMDOMPIN A0 // 随机数seed信号引脚

// 灯带
#define NUMPIXELS 360     // 灯带灯珠数量，1米60珠
#define MAXBRIGHTNESS 255 // 最大亮度，0-255
// #define ISMASTER 0        // 核心云为1，随从云为0

// ==========调节效果变量声明==========
// 七彩
extern uint8_t rainbowDelayTime;
extern uint8_t rainbowCycleTimes;
// extern uint16_t slaveCloudRandomDelayMax;
// 流动
extern uint8_t flowingDelayTime;
extern uint8_t flowingCycleTimes;
extern uint16_t flowingColorRed;
extern uint16_t flowingColorGreen;
extern uint16_t flowingColorBlue;
// 闪电
extern uint8_t lighteningDelayTime;
extern uint8_t lighteningDelayTime2;
extern uint16_t lighteningDelayTime3;
extern uint8_t lighteningCycleTimes;
extern uint16_t lighteningColorRed;
extern uint16_t lighteningColorGreen;
extern uint16_t lighteningColorBlue;
// 清晨
// extern uint16_t firstMorningColorRed;
// extern uint16_t firstMorningColorGreen;
// extern uint16_t firstMorningColorBlue;
extern uint16_t morningDelayUnit;

// ==========其它变量声明==========
extern Adafruit_NeoPixel strip;

extern boolean isExcutingAnyMode;

// ==========函数声明==========
void playCloudLightEffect(byte);
void lightening(uint16_t, uint16_t, uint16_t, uint8_t, uint8_t, uint16_t);
void lighteningSegment(uint16_t, uint16_t, uint16_t, byte, uint16_t);
void flowingCycle(uint16_t, uint16_t, uint16_t, uint8_t);
void rainbowCycle(uint8_t);
uint32_t Wheel(byte);
// void slaveCloudRandomDelay();

// 类声明 1.23
class Cloud
{
    // byte group;
    // byte inGroupNumber;
    // byte chipNumber;
    boolean firstTimeflowingFlag = true;
    uint16_t currentFlowingFirstPos = strip.numPixels() / 4;
    uint16_t currentFlowingLastPos = 0;

    byte morningStage = 0;
    boolean morningEnd = false;

public:
    uint16_t numPixels = strip.numPixels();
    byte mode = 0;

public:
    // void setCloud(byte _group, byte _inGroupNumber, byte _chipNumber)
    // {
    //     group = _group;
    //     chipNumber = _chipNumber;
    // }

    // void clearOtherCloudEffect(byte _thisMode)
    // {
    //     if (_thisMode != 4) // 如果这不是模式4，则把流动的效果清空
    //     {
    //         firstTimeflowingFlag = true;
    //         currentFlowingFirstPos = strip.numPixels() / 4;
    //         currentFlowingLastPos = 0;
    //     }
    // }
    void clearCloudEffects()
    {
       // 清空清晨的阶段
        morningStage = 0;
        morningEnd = false;
        // 清空流动的效果
        firstTimeflowingFlag = true;
        currentFlowingFirstPos = strip.numPixels() / 4;
        currentFlowingLastPos = 0;
    }

    void setCloudMode(byte _mode)
    {
        mode = _mode;
        switch (_mode)
        {
        case 1: // 清晨
            // clearOtherCloudEffect(1);
            morning();
            break;
        case 4: // 流动
            // clearOtherCloudEffect(4);
            flowing(flowingColorRed, flowingColorGreen, flowingColorBlue, 1);
            break;
        }
    }

    void flowing(uint16_t _r, uint16_t _g, uint16_t _b, uint8_t _wait)
    {
        if (firstTimeflowingFlag)
        {
            strip.clear();
            for (uint16_t i = currentFlowingLastPos; i <= currentFlowingFirstPos; i++)
            {
                strip.setPixelColor(i, strip.Color(_r, _g, _b));
                strip.show();
                delay(10);
            }
            firstTimeflowingFlag = false;
        }
        else
        {
            strip.setPixelColor(currentFlowingLastPos, strip.Color(0, 0, 0));
            currentFlowingFirstPos++;
            currentFlowingLastPos++;
            if (currentFlowingFirstPos >= strip.numPixels())
            {
                currentFlowingFirstPos = 0;
            }

            if (currentFlowingLastPos >= strip.numPixels())
            {
                currentFlowingLastPos = 0;
            }

            strip.setPixelColor(currentFlowingFirstPos, strip.Color(_r, _g, _b));
        }

        // delay(_wait);
    }

    void morning()
    {
        // 清晨只执行一次，大云需要大约29s
        if (morningEnd == false)
        {
            switch (morningStage)
            {
            case 0: // 黑->略暗的蓝
                Serial.println();
                Serial.println("---------morning start!!!!---------");
                Serial.print("morningStage:");
                Serial.println(morningStage);
                morningStage = 1;
                changeColorGradually(0, 0, 0, 0, 0, 30, 30, morningDelayUnit);
                delay(morningDelayUnit * 10);
                break;
            case 1: // 略暗的蓝->日出的暖黄
                Serial.print("morningStage:");
                Serial.println(morningStage);
                morningStage = 2;
                changeColorGradually(0, 0, 30, 80, 30, 0, 30, morningDelayUnit / 2);
                delay(morningDelayUnit * 15);
                break;
            case 2: // 日出的暖黄->奶白
                Serial.print("morningStage:");
                Serial.println(morningStage);
                morningStage = 3;
                changeColorGradually(80, 30, 0, 220, 220, 180, 30, morningDelayUnit);
                delay(morningDelayUnit * 20);
                break;
            case 3: // 变黑
                Serial.print("morningStage:");
                Serial.println(morningStage);
                morningStage = 0;
                changeColorGradually(220, 220, 180, 0, 0, 0, 20, morningDelayUnit);
                delay(morningDelayUnit * 5);
                morningEnd = true;
                Serial.println("---------morning end!!!!---------");
                break;
            default:
                break;
            }
        }
    }

    void changeColorGradually(uint16_t _rf, uint16_t _gf, uint16_t _bf, uint16_t _rt, uint16_t _gt, uint16_t _bt, uint16_t _times, uint16_t _delay)
    {
        if (_times == 0)
        {
            _times = 1;
        }

        float rInteval = float(_rt - _rf) / float(_times);
        float gInteval = float(_gt - _gf) / float(_times);
        float bInteval = float(_bt - _bf) / float(_times);

        Serial.print(rInteval);
        Serial.print("\t");
        Serial.print(gInteval);
        Serial.print("\t");
        Serial.println(bInteval);
        float tmpRed = float(_rf);
        float tmpGreen = float(_gf);
        float tmpBlue = float(_bf);

        // boolean rChanged = false;
        // boolean gChanged = false;
        // boolean bChanged = false;

        for (uint16_t i = 0; i < _times; i++)
        {
            // red
            if (rInteval < 0)
            {
                if (tmpRed + rInteval >= _rt)
                {
                    tmpRed += rInteval;
                    if (tmpRed < 0)
                    {
                        tmpRed = 0;
                    }
                }
            }
            else
            {
                if (tmpRed + rInteval <= _rt)
                {
                    tmpRed += rInteval;
                    if (tmpRed > _rt)
                    {
                        tmpRed = _rt;
                    }
                }
            }

            // green
            if (gInteval < 0)
            {
                if (tmpGreen + gInteval >= _gt)
                {
                    tmpGreen += gInteval;
                    if (tmpGreen < 0)
                    {
                        tmpGreen = 0;
                    }
                }
            }
            else
            {
                if (tmpGreen + gInteval <= _gt)
                {
                    tmpGreen += gInteval;
                    if (tmpGreen > _gt)
                    {
                        tmpGreen = _gt;
                    }
                }
            }

            // blue
            if (bInteval < 0)
            {
                if (tmpBlue + bInteval >= _bt)
                {
                    tmpBlue += bInteval;
                    if (tmpBlue < 0)
                    {
                        tmpBlue = 0;
                    }
                }
            }
            else
            {
                if (tmpBlue + bInteval <= _bt)
                {
                    tmpBlue += bInteval;
                    if (tmpBlue > _bt)
                    {
                        tmpBlue = _bt;
                    }
                }
            }

            Serial.print(tmpRed);
            Serial.print("\t");
            Serial.print(tmpGreen);
            Serial.print("\t");
            Serial.println(tmpBlue);
            // changeAllColor(uint16_t(tmpRed), uint16_t(tmpGreen), uint16_t(tmpBlue));
            strip.fill(strip.Color(uint16_t(tmpRed), uint16_t(tmpGreen), uint16_t(tmpBlue)));
            strip.show();
            delay(_delay);
        }
    }

    // void changeAllColor(uint16_t _r, uint16_t _g, uint16_t _b)
    // {

    // for (uint16_t i = 0; i < strip.numPixels(); i++)
    // {
    //     strip.setPixelColor(i, strip.Color(_r, _g, _b));
    // }
    // }
};

extern Cloud cloud;
