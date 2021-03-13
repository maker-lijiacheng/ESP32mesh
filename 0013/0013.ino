#define BLYNK_PRINT Serial
#include <SoftwareSerial.h>
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include "config.h"
#include "keys.h"

// ==========调节效果变量定义==========
// uint16_t brightness = 255;
#define MICROWAVEPIN 13 //微波传感器管脚

// char auth[] = "sbv5XPRtDWoOhwVqLJyx2KqWYtXox4Gy";
// char ssid[] = "xixi"; //"R101i-AAE876";
// char pass[] = "qwerty123456";

// 七彩
uint8_t rainbowDelayTime = 20; // 七彩延迟时间=循环快慢，最大127
uint8_t rainbowCycleTimes = 1; // 七彩循环次数，最大127
// uint16_t slaveCloudRandomDelayMax = 0; //调节小云出现的随机延迟时间的最大值
// 流动
uint8_t flowingDelayTime = 100;   // 流动延迟时间=循环快慢，最大127
uint8_t flowingCycleTimes = 1;    // 流动循环次数，最大127
uint16_t flowingColorRed = 10;    // 颜色
uint16_t flowingColorGreen = 120; //
uint16_t flowingColorBlue = 10;   //
// 闪电
uint8_t lighteningDelayTime = 50;     // 闪电左右之间的延迟时间，最大127
uint8_t lighteningDelayTime2 = 100;   // 闪电两下之间的延迟时间，最大127
uint16_t lighteningDelayTime3 = 4000; // 闪电和闪电之间的延迟时间
uint8_t lighteningCycleTimes = 1;     // 闪电循环次数，最大127
uint16_t lighteningColorRed = 200;    // 颜色
uint16_t lighteningColorGreen = 180;  //
uint16_t lighteningColorBlue = 180;   //
// 清晨
uint16_t morningDelayUnit = 200;

WidgetLED ledV10(V10); //运行指示灯
SoftwareSerial mySerial(13, 14);
DFRobotDFPlayerMini myPlayer;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, STRIPPIN, NEO_GRB + NEO_KHZ800);
Cloud cloud;

volatile int ws2812_R;
volatile int ws2812_G;
volatile int ws2812_B;
volatile int F;
volatile int mode;
volatile int speed;
volatile int run_flag;
volatile int mp3num;
volatile int num;

WidgetBridge bridgeV16(V16);
WidgetBridge bridgeV17(V17);
WidgetBridge bridgeV18(V18);
WidgetBridge bridgeV19(V19);
WidgetBridge bridgeV20(V20);
WidgetBridge bridgeV21(V21);
WidgetBridge bridgeV22(V22);
WidgetBridge bridgeV23(V23);
WidgetBridge bridgeV24(V24);
WidgetBridge bridgeV25(V25);
WidgetBridge bridgeV26(V26);
WidgetBridge bridgeV27(V27);
WidgetBridge bridgeV28(V28);
WidgetBridge bridgeV29(V29);
WidgetBridge bridgeV30(V30);
WidgetBridge bridgeV31(V31);
WidgetBridge bridgeV32(V32);
WidgetBridge bridgeV33(V33);
WidgetBridge bridgeV34(V34);
WidgetBridge bridgeV35(V35);
WidgetBridge bridgeV36(V36);
WidgetBridge bridgeV37(V37);
WidgetBridge bridgeV38(V38);
WidgetBridge bridgeV39(V39);
WidgetBridge bridgeV40(V40);
WidgetBridge bridgeV41(V41);

WidgetBridge bridgeV42(V42); // 是否执行中
WidgetBridge bridgeV43(V43); // 是否执行中
WidgetBridge bridgeV44(V44); // 是否执行中
WidgetBridge bridgeV45(V45); // 是否执行中
WidgetBridge bridgeV46(V46); // 是否执行中

int vpin0_value;
int vpin1_value;
int vpin2_value;
int vpin3_value;
int vpin4_value;
int vpin5_value;
int vpin6_value;
int vpin7_value;
int vpin11_value;
int vpin12_value;
int vpin100_value;

int time_flag = 0;
// int old_second = 0;
int runmode = 5;
long lastMoveTime = -30000; //记录下有人运动时的时间，初始值为-30s
long mode6LastPlayMP3Time = -30000;
long lastSyncTime = 0;
int moveflag = 0; // 有人1  无人0
boolean isExcutingAnyMode = false;
boolean hasExcutedEnd = false;
boolean playMP3WhenMode6 = false;

WidgetRTC rtc;
BlynkTimer timer;

BLYNK_WRITE(V100)
{
  vpin100_value = param.asInt();
  if (!isExcutingAnyMode)
  {
    strip.updateLength(vpin100_value * 60);
  }
}

BLYNK_WRITE(V0)
{
  vpin0_value = param.asInt();
  ws2812_R = vpin0_value;
}

BLYNK_WRITE(V1)
{
  vpin1_value = param.asInt();
  ws2812_G = vpin1_value;
}

BLYNK_WRITE(V2)
{
  vpin2_value = param.asInt();
  ws2812_B = vpin2_value;
}

BLYNK_WRITE(V3)
{
  vpin3_value = param.asInt();
  mode = vpin3_value;
}

BLYNK_WRITE(V4)
{
  vpin4_value = param.asInt();
  if (vpin4_value == 1)
  {
    playMP3WhenMode6 = true;
    myPlayer.play(mp3num);
    Serial.print("run");
    Serial.print(mp3num);
    Serial.println(".mp3");
  }
}

BLYNK_WRITE(V5)
{
  vpin5_value = param.asInt();
  if (vpin5_value == 1)
  {
    playMP3WhenMode6 = false;
    myPlayer.pause();
    Serial.println("stop all mp3");
  }
}

BLYNK_WRITE(V6)
{
  vpin6_value = param.asInt();
  mp3num = vpin6_value;
}
BLYNK_WRITE(V7)
{
  vpin7_value = param.asInt();
  morningDelayUnit = vpin7_value;
}

BLYNK_WRITE(V11)
{
  vpin11_value = param.asInt();
  myPlayer.volume(vpin11_value);
}

BLYNK_WRITE(V12)
{
  vpin12_value = param.asInt();
  strip.setBrightness(vpin12_value);
}

BLYNK_APP_DISCONNECTED()
{
  Serial.println("APP_DISCONNECTED");
  time_flag = 1;
}

BLYNK_APP_CONNECTED()
{
  Serial.println("APP_CONNECTED");
}
void myTimerEvent1()
{
  checkConnection();
  run_flag = run_flag + 1;
  if (run_flag % 2 == 1)
  {
    ledV10.on();
  }
  else if (run_flag % 2 == 0)
  {
    ledV10.off();
  }
  if (run_flag >= 100)
  {
    run_flag = 0;
  }
}

void checkConnection()
{
  if (WiFi.status() == 6 || Blynk.connected() == 0)
  {
    strip.clear(); //全黑
    strip.show();  //显示生效
    delay(100);

    Serial.println("诶！我怎么断线了！！！！重生ing~~~~~~~~~~~~~~~");
    Serial.println("诶！我怎么断线了！！！！重生ing~~~~~~~~~~~~~~~");
    Serial.println("诶！我怎么断线了！！！！重生ing~~~~~~~~~~~~~~~");
    ESP.restart(); //软复位，重连路由器
  }
}

void mp3play()
{
  if (mode >= 1 && mode <= 4)
  {
    myPlayer.play(mode);
    // Serial.println("开始播放MP3~~~");
    // Serial.print("run：");
    // Serial.print(mode);
    // Serial.println(".mp3");
  }
}

void syncExcutionStateToCloud()
{
  bridgeV42.virtualWrite(V17, isExcutingAnyMode);
  bridgeV43.virtualWrite(V17, isExcutingAnyMode);
  bridgeV44.virtualWrite(V17, isExcutingAnyMode);
  bridgeV45.virtualWrite(V17, isExcutingAnyMode);
  bridgeV46.virtualWrite(V17, isExcutingAnyMode);
}

void syncModeToCloud(int _mode)
{
  bridgeV19.virtualWrite(V3, _mode);
  bridgeV24.virtualWrite(V3, _mode);
  bridgeV29.virtualWrite(V3, _mode);
  bridgeV34.virtualWrite(V3, _mode);
  bridgeV39.virtualWrite(V3, _mode);
}

void syncParaToCloud()
{
  bridgeV16.virtualWrite(V0, ws2812_R); //同步
  bridgeV17.virtualWrite(V1, ws2812_G);
  bridgeV18.virtualWrite(V2, ws2812_B);
  bridgeV20.virtualWrite(V4, morningDelayUnit);
  bridgeV21.virtualWrite(V0, ws2812_R); //同步
  bridgeV22.virtualWrite(V1, ws2812_G);
  bridgeV23.virtualWrite(V2, ws2812_B);
  bridgeV25.virtualWrite(V4, morningDelayUnit);
  bridgeV26.virtualWrite(V0, ws2812_R); //同步
  bridgeV27.virtualWrite(V1, ws2812_G);
  bridgeV28.virtualWrite(V2, ws2812_B);
  bridgeV30.virtualWrite(V4, morningDelayUnit);
  bridgeV31.virtualWrite(V0, ws2812_R); //同步
  bridgeV32.virtualWrite(V1, ws2812_G);
  bridgeV33.virtualWrite(V2, ws2812_B);
  bridgeV35.virtualWrite(V4, morningDelayUnit);
  bridgeV36.virtualWrite(V0, ws2812_R); //同步
  bridgeV37.virtualWrite(V1, ws2812_G);
  bridgeV38.virtualWrite(V2, ws2812_B);
  bridgeV40.virtualWrite(V4, morningDelayUnit);
}

void syncModeToWind(int _mode)
{

  bridgeV41.virtualWrite(V5, _mode); //风机}
}

void authEachCloudAndWind()
{
  bridgeV16.setAuthToken(authSonCloud1);
  bridgeV17.setAuthToken(authSonCloud1);
  bridgeV18.setAuthToken(authSonCloud1);
  bridgeV19.setAuthToken(authSonCloud1);
  bridgeV20.setAuthToken(authSonCloud1);
  bridgeV21.setAuthToken(authSonCloud2);
  bridgeV22.setAuthToken(authSonCloud2);
  bridgeV23.setAuthToken(authSonCloud2);
  bridgeV24.setAuthToken(authSonCloud2);
  bridgeV25.setAuthToken(authSonCloud2);
  bridgeV26.setAuthToken(authSonCloud3);
  bridgeV27.setAuthToken(authSonCloud3);
  bridgeV28.setAuthToken(authSonCloud3);
  bridgeV29.setAuthToken(authSonCloud3);
  bridgeV30.setAuthToken(authSonCloud3);
  bridgeV31.setAuthToken(authSonCloud4);
  bridgeV32.setAuthToken(authSonCloud4);
  bridgeV33.setAuthToken(authSonCloud4);
  bridgeV34.setAuthToken(authSonCloud4);
  bridgeV35.setAuthToken(authSonCloud4);
  bridgeV36.setAuthToken(authSonCloud5);
  bridgeV37.setAuthToken(authSonCloud5);
  bridgeV38.setAuthToken(authSonCloud5);
  bridgeV39.setAuthToken(authSonCloud5);
  bridgeV40.setAuthToken(authSonCloud5);
  bridgeV41.setAuthToken(authWind);

  bridgeV42.setAuthToken(authSonCloud1);
  bridgeV43.setAuthToken(authSonCloud2);
  bridgeV44.setAuthToken(authSonCloud3);
  bridgeV45.setAuthToken(authSonCloud4);
  bridgeV46.setAuthToken(authSonCloud5);
}

void runMode(int _mode)
{
  // 1:清晨 2：闪电 3.七彩 4.流绿 5.变暗 6.亮色
  switch (_mode)
  {
    case 1: // 清晨
      cloud.setCloudMode(1);
      break;

    case 2: // 闪电
      playCloudLightEffect(2);
      break;

    case 3: // 七彩
      playCloudLightEffect(3);
      break;

    case 4: // 新的流动
      cloud.setCloudMode(4);
      break;

    case 5: // 关闭
      for (int i = 1; i <= strip.numPixels(); i = i + (1))
      {
        strip.setPixelColor((i) - 1, 0x000000);
      }
      break;
    case 6: // 亮色
      for (int i = 1; i <= strip.numPixels(); i = i + (1))
      {
        strip.setPixelColor((i) - 1, ((ws2812_R & 0xffffff) << 16) | ((ws2812_G & 0xffffff) << 8) | ws2812_B);
      }
      break;
    default:
      break;
  }
  strip.show();
}

void setup()
{
  strip.begin();
  strip.clear();
  strip.show();

  mySerial.begin(9600);
  myPlayer.begin(mySerial);

  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass, "blynk.mixly.org", 8080);
  mode = 0;
  speed = 100;
  mp3num = 1;
  num = 0;
  ledV10.setColor("#66ff99");
  myPlayer.volume(15);      //初始音量等级，范围0-30
  run_flag = 0;             //通过V10实时返回变化的数据，灯闪烁代表设备在线
  strip.setBrightness(20);  //设置灯带初始亮度值
  rtc.begin();              //实时时间RTC组件初始化
  setSyncInterval(10 * 60); //同步时间，间隔十分钟
  Serial.println(Blynk.connected());
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH); //8位IO口电平转换模块使能管脚，高电平工作
  pinMode(13, INPUT_PULLUP);

  //Blynk.virtualWrite(V0, 0);//发送到APP虚拟管脚
  //bridgeV28.virtualWrite(V0, 0);//桥接其他设备虚拟管脚
  timer.setInterval(500L, myTimerEvent1);
  Blynk.run();

  authEachCloudAndWind();
  syncModeToCloud(5);
  runmode = 5;
  // cloud.setCloud(1, 1, 1);
}

void loop()
{
  Blynk.syncAll();
  Blynk.run();
  timer.run();

  // Serial.println();
  // Serial.print("当前时间：");
  // Serial.print(millis());
  // Serial.print("\t lastMoveTime:");
  // Serial.print(lastMoveTime);
  // Serial.print("\t millis() - lastMoveTime:");
  // Serial.print(millis() - lastMoveTime);
  // Serial.print("\t excuting:");
  // Serial.println(isExcutingAnyMode);

  //  Serial.println(Blynk.connected());//服务器连接状态   1
  //  Serial.println(WiFi.status());//wifi状态   3连接 6掉线
  //  Serial.println(digitalRead(MICROWAVEPIN));//微波传感器管脚

  // checkConnection(); //检测是否离线，掉线则重启
  attachInterrupt(digitalPinToInterrupt(13), attachInterrupt_fun_13, FALLING);

  //Serial.println(second());
  if (minute() % 5 == 0 && second() >= 50) // 5分钟自动重启，于每个5分钟的最后10s。原则上10s内重启过一次则不需要重启了，但还没有实现这个功能。5/6模式不需要重启
  {

    // Serial.println("===================================================");
    // Serial.println("===================================================");
    // Serial.println("================5分钟到了，毁灭吧！！！===============");
    // Serial.println("===================================================");
    // Serial.println("===================================================");
    strip.clear(); //全黑
    strip.show();  //显示生效
    delay(100);
    ESP.restart(); //软复位，重连路由器
  }

  if (mode != 5 && mode != 6)
  {
    if (!isExcutingAnyMode) // 不执行效果的时候，可以定时同步数据或者定时重启
    {
      if (millis() - lastSyncTime > 2000) //每隔2秒执行一次，执行效果的过程中，不进行同步
      {
        Serial.println("同步ing");
        syncParaToCloud();
        syncExcutionStateToCloud();
        // syncModeToCloud(mode);
        num = 0;                 //重置微波传感器计数器
        lastSyncTime = millis(); //记录当前时间
      }


    }

    //13号管脚 微波传感器 下降沿触发 有人移动时为0，无人时为1
    if (moveflag == 1)
    {
      moveflag = 0; // 又开始可以检测了
      Serial.println("检测到人移动");
      if (millis() - lastMoveTime > 30000) //如果距离上一次已经过去30s
      {

        Serial.println("距离上一次触发已经过去30s，避免重复触发");

        lastMoveTime = millis(); //记录下当前时间，开始move的时刻
        Serial.print("记录开始执行的movetime:");
        Serial.println(lastMoveTime);

        runmode = mode;
        // 先出声音，再出灯，再出风
        mp3play();

        delay(3000);

        // 过一会再传给小云，开始亮灯
        Serial.println("开始喊大家一起亮灯");
        syncModeToCloud(mode);
        runMode(runmode);

        // 传给风机，风机过一会开始吹风
        Serial.println("开始吹风");
        syncModeToWind(mode);

        isExcutingAnyMode = true;
        syncExcutionStateToCloud();

        hasExcutedEnd = false;
      }
    }
    else
    {
      // moveflag == 0 没有人
      // Serial.println("没有检测到移动");
      if (millis() - lastMoveTime > 30000) //如果距离上一次已经过去30s，效果该结束了
      {
        if (!hasExcutedEnd)
        {
          Serial.println("上一个效果已经持续30秒，可以闭眼了");
          //先关风机
          syncModeToWind(0);
          delay(2000);
          //过一会再关灯
          syncModeToCloud(5);
          isExcutingAnyMode = false;
          syncExcutionStateToCloud();
          runmode = 5; //不亮模式
          runMode(runmode);
          cloud.clearCloudEffects(); // 清空云效果模式参数

          delay(2000); //过2秒，音乐暂停
          myPlayer.pause();

          //    ESP.restart();//软复位，重连路由器
          hasExcutedEnd = true;
        }
        else
        {
          // Serial.println("....闭...眼...中......休...息...");
          myPlayer.pause();
        }
      }
      else
      {
        // 还没有到30s
        runMode(runmode);
      }
    }
  }
  else
  {
    runmode = mode;
    runMode(runmode);
    syncParaToCloud();
    syncModeToWind(0);
    syncModeToCloud(mode);
    if (mode == 5)
    {
      myPlayer.pause();
    }
    else if (mode == 6)
    {
      if (playMP3WhenMode6)
      {
        if (millis() - mode6LastPlayMP3Time > 600000)
        {
          //myPlayer.pause();
          myPlayer.play((minute() % 4) + 1);
          mode6LastPlayMP3Time = millis();
        }
      }
      else
      {
        myPlayer.pause();
      }
    }
  }
}

ICACHE_RAM_ATTR void attachInterrupt_fun_13()
{ //微波传感器管脚硬件中断
  num = num + 1;

  Serial.print(num); //微波传感器管脚

  if (num >= 25)
  { //大概是两次挥手
    num = 0; //重置计数器
    if (mode >= 1 && mode <= 4)
    {
      moveflag = 1;
    }
  }
}
