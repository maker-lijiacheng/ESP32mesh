#include <SoftwareSerial.h>
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <Adafruit_NeoPixel.h>
#include "painlessMesh.h"
#include <SimpleTimer.h>

#define   MESH_PREFIX     "clouds"
#define   MESH_PASSWORD   "sattvalab"
#define   MESH_PORT       5555
Scheduler userScheduler;  // to control your personal task 创建子线程
painlessMesh  mesh;       // 定义mesh联网

#define PIN           12  // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS     600 // Popular NeoPixel ring size
#define MICROWAVEPIN  16  //微波传感器管脚
#define BRIGHTNES     20  //灯带亮度设置
void sendMessage(); // Prototype so PlatformIO doesn't complain

SoftwareSerial mySerial(13, 14);  //MP3模块软串口管脚
DFRobotDFPlayerMini myPlayer;     //定义MP3模块
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);//初始化灯带

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage ); //创建一个子任务 间隔为一秒执行一次（初始化发送时间间隔）

int run_mode = 0; //云朵运行的模式值
int run_flag = 0 ;//微波传感器有人标志
int run_number = 0;//有人状态时loop()运行次数
int begin_time = 0 ;//有人状态时的时间标志位
int normal_time = 0 ;//无人状态时的时间标志位

void sendMessage() 
{ //发送一条字符串
  String msg = "Hello from node ";
  msg += mesh.getNodeId();//字符串末尾添加信息来源
  //msg = msg + " Humidity: " + String(dht.readHumidity());//发送湿度示例
  mesh.sendBroadcast( msg );//字符串内容
  //taskSendMessage.setInterval(200);//重新设置发送间隔，单位ms
}

void receivedCallback( uint32_t from, String &msg ) {//收到消息 （ID，字符串）
    Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {//mesh网络中检测到新节点，并读取nodeID值
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}  

void changedConnectionCallback() {//mesh网络中发生变动
    Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {//同步mesh时间戳
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void colorWipe(uint32_t color, int wait) {// 按顺序显示单种颜色 从头逐渐亮到尾
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void colorWipeAll(uint32_t color) {// 显示单种颜色 一次性刷新
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    }
    strip.show();                          //  Update strip to match
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void Mode_1_Morning()//清晨模式 单步运行
{
  
}
void Mode_1_Morning_Init()//清晨模式 初始化
{
  
}
void Mode_2_Lightening()//闪电模式 单步运行
{
  
}
void Mode_2_Lightening_Init()//闪电模式 初始化
{
  
}
void Mode_3_Morning()//七彩模式 单步运行
{
  
}
void Mode_3_Morning_Init()//七彩模式 初始化
{
  
}
void Mode_4_Morning()//流动模式 单步运行
{
  
}
void Mode_4_Morning_Init()//流动模式 初始化
{
  
}
void Mode_Nobody()//无人触发模式 随机一种颜色   *注意，执行一次就会重新生成一种颜色 
{
  colorWipeAll(strip.Color(random(0,255),random(0,255),random(0,255)));
  strip.show();
}
void Mode_Stop()//所有灯带熄灭
{
  strip.clear();
  strip.show();
}

/****************************************************************/
/****************************************************************/
void setup() {
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif
  
  Serial.begin(115200);//USB串口波特率
  mySerial.begin(9600);//MP3模块串口波特率
  myPlayer.begin(mySerial);//MP3模块初始化
  //myPlayer.play(1);//播放音乐，第X首  

  strip.begin();//WS2812初始化
  strip.setBrightness(BRIGHTNES); // Set BRIGHTNESS to about 1/5 (max = 255)
  strip.clear();//灯带全黑
  strip.show();//效果生效
  
  mesh.setDebugMsgTypes( ERROR | STARTUP );

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );//初始化mesh网络
  mesh.onReceive(&receivedCallback);                      //收到一条群发消息，返回消息值
  mesh.onNewConnection(&newConnectionCallback);           //mesh网络中检测到新节点，并读取nodeID值
  mesh.onChangedConnections(&changedConnectionCallback);  //mesh网络中发生变动
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);     //同步mesh时间戳

  userScheduler.addTask(taskSendMessage);   //新建子线程，可用于数据传输
  taskSendMessage.enable();                 //子线程使能
/* 灯带基础显示程序
  colorWipe(strip.Color(  0,   0, 255), 5); // Blue
  colorWipeAll(strip.Color(  0,   0, 255));
  rainbow(20);
  rainbowCycle(20);
  */
}

void loop(){
  mesh.update();    //尽可能让这句话频繁运行
  attachInterrupt(digitalPinToInterrupt(MICROWAVEPIN), attachInterrupt_fun, FALLING);   //设置微波传感器管脚为中断下降沿触发
  if(run_flag==1){ // 开始运行标志
    run_number = run_number + 1 ; //标志模式运行次数，每次loop自加1
    begin_time = millis();        //记录当前的开始时间
  }
  if(millis() - begin_time > 15000 && run_flag == 1 ){//当前模式执行15秒以后停止
    Mode_Nobody();  //随机一种颜色
    run_flag = 0;   // 无人运行标志
  }
  
  switch (run_mode)
  {
    case 1:   Mode_1_Morning();    myPlayer.play(1);  break;
    case 2:   Mode_2_Lightening(); myPlayer.play(2);  break;
    case 3:   Mode_3_Morning();    myPlayer.play(3);  break;
    case 4:   Mode_4_Morning();    myPlayer.play(4);  break;
    default:  Mode_Stop();  break;   //其他数值（刚开机）处于关闭状态
  }
}

ICACHE_RAM_ATTR void attachInterrupt_fun()
{ //微波传感器管脚硬件中断
  Serial.println("ESP8266 GPIO16 FALLING");
  if(run_flag == 0){//从无人状态切换到动态效果时，做一次运行次数归零
    run_number = 0;
  }
  run_flag = 1 ;// 开始执行效果标志，0：无人   1：有人触发
}

/*colorWipe(strip.Color(255,   0,   0), 5); // Red
  strip.clear();
  colorWipe(strip.Color(  0, 255,   0), 5); // Green
  strip.clear();
  colorWipe(strip.Color(  0,   0, 255), 5); // Blue
  strip.clear();
  colorWipeAll(strip.Color(  255,   0, 0));
  delay(1000);
  colorWipeAll(strip.Color(  0,   255, 0));
  delay(1000);
  colorWipeAll(strip.Color(  0,   0, 255));
  delay(1000);
  rainbow(20);             // Flowing rainbow cycle along the whole strip
  delay(2000);
  rainbowCycle(20);   */
