#include <SoftwareSerial.h>
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <Adafruit_NeoPixel.h>
#include "painlessMesh.h"
#include <SimpleTimer.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
Scheduler userScheduler;  // to control your personal task 创建子线程
painlessMesh  mesh;       // 定义mesh联网

#define PIN           12  // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS     600 // Popular NeoPixel ring size
#define MICROWAVEPIN  16  //微波传感器管脚
#define BRIGHTNES     255  //灯带亮度设置 0~255

void sendMessage(); // Prototype so PlatformIO doesn't complain
void attachInterrupt_fun();

SoftwareSerial mySerial(13, 14);  //MP3模块软串口管脚
DFRobotDFPlayerMini myPlayer;     //定义MP3模块
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);//初始化灯带

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage ); //创建一个子任务 间隔为一秒执行一次（初始化发送时间间隔）

int mp3_volume = 20;//MP3模块音量值
int run_mode = 0; //云朵运行的模式值
int old_mode = 0;//记录上次的模式
int run_flag = 0 ;//微波传感器有人标志
int run_number = 0;//有人状态时loop()运行次数
int begin_time = 0 ;//有人状态时的时间标志位
int normal_time = 0 ;//无人状态时的时间标志位
int run_time_limit = 5000;//每种模式的运行时间 15s

int mode_init_flag = 0;//模式参数初始化标志位 0：可复位 1：不可复位 无人时会归0

int send_r = 0; //有人时，随机生成3个RGB值，并发送给其他云
int send_g = 0;
int send_b = 0;

int read_r = 0; //从其他云接收到的RGB值
int read_g = 0;
int read_b = 0;
int read_mode = 0;//从其他云接收到的mode值

void sendMessage() 
{ //发送一条字符串
  String msg = "";
  //msg += mesh.getNodeId();//字符串末尾添加信息来源
  msg = msg + "R" + String(send_r) + "G" + String(send_g) + "B" + String(send_b);
  msg = msg + "M" + String(run_mode);//发送mode值给其他云 0：无人模式 1~4:4种运行模式
  // msg = R255G255B255M2  
  mesh.sendBroadcast( msg );//向网络中发送msg字符串
  //taskSendMessage.setInterval(200);//重新设置发送间隔，单位ms
}

void receivedCallback( uint32_t from, String &msg ) {//收到消息 （ID，字符串）
    Serial.printf("ID:%u MSG:%s\n", from, msg.c_str());
    //解析字符串
    read_r = String(msg.substring(msg.indexOf("R")+1,msg.indexOf("G"))).toInt();//截取并转为int类型
    read_g = String(msg.substring(msg.indexOf("G")+1,msg.indexOf("B"))).toInt();
    read_b = String(msg.substring(msg.indexOf("B")+1,msg.indexOf("M"))).toInt();
    read_mode = String(msg.substring(msg.indexOf("M")+1,msg.indexOf("M")+2)).toInt();
    //打印
    Serial.print("R:");Serial.print(read_r);Serial.print("  ");//串口返回RGB值
    Serial.print("G:");Serial.print(read_g);Serial.print("  ");
    Serial.print("B:");Serial.print(read_b);Serial.print("  ");
    Serial.print("M:");Serial.print(read_mode);Serial.println("  ");
    
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
//此处除了每次loop自加的run_number值，其他尽量用局部变量（Init()用来设置初值，）
void Mode_1_Morning()//清晨模式 单步运行
{
  if(mode_init_flag == 0){//只复位一次
    Mode_1_Morning_Init();
    mode_init_flag = 1;
  }
  
}
void Mode_1_Morning_Init()//清晨模式 初始化
{
  Mode_Stop();//熄灭所有灯
  myPlayer.play(1);//播放音乐
}
void Mode_2_Lightening()//闪电模式 单步运行
{
  if(mode_init_flag == 0){//只复位一次
    Mode_2_Lightening_Init();
    mode_init_flag = 1;
  }
}
void Mode_2_Lightening_Init()//闪电模式 初始化
{
  Mode_Stop();//熄灭所有灯
  myPlayer.play(2);//播放音乐
}
void Mode_3_rainbow()//七彩模式 单步运行
{
  if(mode_init_flag == 0){//只复位一次
    Mode_3_rainbow_Init();
    mode_init_flag = 1;
  }
}
void Mode_3_rainbow_Init()//七彩模式 初始化
{
  Mode_Stop();//熄灭所有灯  
  myPlayer.play(3);//播放音乐
}
void Mode_4_flowing()//流动模式 单步运行
{
  if(mode_init_flag == 0){//只复位一次
    Mode_4_flowing_Init();
    mode_init_flag = 1;
  }
}
void Mode_4_flowing_Init()//流动模式 初始化
{
  Mode_Stop();//熄灭所有灯  
  myPlayer.play(4);//播放音乐
}
void Mode_Nobody()//无人触发模式 随机一种颜色   *注意，执行一次就会重新生成一种颜色 
{
  colorWipeAll(strip.Color(send_r,send_g,send_b));
  strip.show();
  myPlayer.pause();
}
void Mode_Stop()//所有灯带熄灭
{ 
  strip.clear();//灯带全黑
  strip.show();//效果生效
}

/****************************************************************/
/****************************************************************/
void setup() {
  Serial.begin(115200);//USB串口波特率
  mySerial.begin(9600);//MP3模块串口波特率
  myPlayer.begin(mySerial);//MP3模块初始化
  myPlayer.volume(mp3_volume); //MP3模块音量大小

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
  
  pinMode(MICROWAVEPIN, INPUT);
  
//  16号管脚不支持中断触发 
//  pinMode(MICROWAVEPIN, INPUT_PULLUP);
//  attachInterrupt(digitalPinToInterrupt(MICROWAVEPIN), attachInterrupt_fun, FALLING);   //设置微波传感器管脚为中断下降沿触发

/* 灯带基础显示程序
  colorWipe(strip.Color(  0,   0, 255), 5); // Blue
  colorWipeAll(strip.Color(  0,   0, 255));
  rainbow(20);
  rainbowCycle(20);
  */
}

void loop(){
  mesh.update();    //尽可能让这句话频繁运行
  
  if(digitalRead(MICROWAVEPIN)==0 && run_flag == 0){//无效果运行时，微波传感器被触发 
    attachInterrupt_fun();//触发处理程序
    run_flag = 1 ;// 开始执行效果标志，0：无人   1：有人触发
  }
  
  if(run_flag==1){ // 开始运行标志
    begin_time = millis();        //记录当前的开始时间
    run_flag = 2;                 //2代表开始执行动态效果
  }
  
  if(run_flag==2){ // 自加计数程序
    run_number = run_number + 1 ; //标志模式运行次数，每次loop自加1
  }
  
  if(millis() - begin_time > run_time_limit && run_flag == 2 ){//当前模式执行15秒以后停止
    run_flag = 0;         // 无人运行标志 使用系统运行时间来结束当前效果
    old_mode = run_mode;  //记录上次的模式
    run_mode = 0;         // 无人运行模式
    mode_init_flag = 0;   //模式复位标志位
    Mode_Nobody(); //无人时显示一种随机颜色
    Serial.println("MICROWAVEPIN READY!!!");  
  }
  
  switch (run_mode)
  { 
    case 1:   Mode_1_Morning();      break;
    case 2:   Mode_2_Lightening();   break;
    case 3:   Mode_3_rainbow();      break;
    case 4:   Mode_4_flowing();      break;
  }
}

//ICACHE_RAM_ATTR void attachInterrupt_fun()//微波传感器管脚硬件中断
void attachInterrupt_fun()
{ 
  Serial.println("ESP8266 GPIO16 FALLING!!!");
//Serial.println(digitalRead(MICROWAVEPIN));//微波传感器管脚
  
  if(run_flag == 0){//从无人状态切换到动态效果时，做一次运行次数归零
    run_number = 0;
    do{
      run_mode = random(1,5);
    }while(run_mode == old_mode);//随机1~4mode，并群发给其他云
    do{
      send_r = random(0,255);
      send_g = random(0,255);
      send_b = random(0,255);
    }while(!(send_r >= 200 || send_g >= 200 || send_b >= 200));//如果随机出暗色，就再重新取一次值
  }
  Serial.print("R:");Serial.print(send_r);Serial.print("  ");//串口返回RGB值
  Serial.print("G:");Serial.print(send_g);Serial.print("  ");
  Serial.print("B:");Serial.print(send_b);Serial.print("  ");
  Serial.print("M:");Serial.print(run_mode);Serial.println("  ");
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
