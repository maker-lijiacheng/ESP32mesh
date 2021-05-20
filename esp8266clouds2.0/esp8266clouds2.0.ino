#include "Arduino.h"
#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"
#include <Adafruit_NeoPixel.h>
#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
Scheduler userScheduler;  // to control your personal task 创建子线程
painlessMesh  mesh;       // 定义mesh联网

#define PIN           12  //灯带管脚
#define NUMPIXELS     600 //灯带数量    //NUMPIXELS=strip.numPixels();
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
int run_time_limit = 15000;//每种模式的运行时间 15s

int mp3_begin_time = 0 ;//无人状态时的音乐开始时间标志位
int mp3_begin_time = 0 ;//无人状态时的音乐开始时间标志位

int nobody_old_mp3num = 0 ;//无人模式时，旧的音乐序号
int nobody_new_mp3num = 0 ;//无人模式时，新生成音乐序号
int mp3_between_time = 22000 ;//无人模式时，两首音乐中的间隔

int mode_init_flag = 0;//模式参数初始化标志位 0：可复位 1：复位完成 无人时会归0

int Mode_3_R = 0; 
int Mode_3_G = 0; 
int Mode_3_B = 0; 
int Mode_3_H = 0; 
int Mode_3_i = 0;
int Mode_3_oldtime = 0;

int send_r = 0; //有人时，随机生成3个RGB值，并发送给其他云
int send_g = 0;
int send_b = 0;
int groups = 1;//定义每组云的分组信息  1、2、3、4  共四组云

int read_r = 0; //从其他云接收到的RGB值
int read_g = 0;
int read_b = 0;
int read_groups = 0;//读取分组信息
int read_mode = 0;//从其他云接收到的mode值


void sendMessage() //每隔一秒发送一次数据
{ //发送一条字符串
  String msg = "";
  //msg += mesh.getNodeId();//字符串末尾添加信息来源
  msg = msg + "R" + String(send_r) + "G" + String(send_g) + "B" + String(send_b);
  msg = msg + "S" + String(groups);//
  msg = msg + "M" + String(run_mode);//发送mode值给其他云 0：无人模式 1~4:4种运行模式
  // msg = R255G255B255S1M2    
  mesh.sendBroadcast( msg );//向网络中发送msg字符串
  //taskSendMessage.setInterval(200);//重新设置发送间隔，单位ms
}

void receivedCallback( uint32_t from, String &msg ) {//收到消息 （ID，字符串）
//读取mesh组网中的公共消息
  Serial.printf("ID:%u MSG:%s\n", from, msg.c_str());
  // 解析字符串
  read_r = String(msg.substring(msg.indexOf("R")+1,msg.indexOf("G"))).toInt();//截取并转为int类型
  read_g = String(msg.substring(msg.indexOf("G")+1,msg.indexOf("B"))).toInt();
  read_b = String(msg.substring(msg.indexOf("B")+1,msg.indexOf("S"))).toInt();
  read_groups = String(msg.substring(msg.indexOf("S")+1,msg.indexOf("M"))).toInt();
  read_mode = String(msg.substring(msg.indexOf("M")+1,msg.indexOf("M")+2)).toInt();
  // 打印
  Serial.print("R:");Serial.print(read_r);Serial.print("  ");//串口返回RGB值
  Serial.print("G:");Serial.print(read_g);Serial.print("  ");
  Serial.print("B:");Serial.print(read_b);Serial.print("  ");
  Serial.print("S:");Serial.print(read_groups);Serial.print("  ");
  Serial.print("M:");Serial.print(read_mode);Serial.println("  ");
}

void newConnectionCallback(uint32_t nodeId) {//mesh网络中检测到新节点，并读取nodeID值
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}  

void changedConnectionCallback() {//mesh网络中发生变动
    Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {// 同步mesh时间戳
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void colorWipe(uint32_t color, int wait) { //  按顺序显示单种颜色 从头逐渐亮到尾
  for(int i=0; i<strip.numPixels(); i++) { //  For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void colorWipeAll(uint32_t color) {// 显示单种颜色 一次性刷新
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      strip.setPixelColor(i, color);       //  Set pixel's color (in RAM)
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
  myPlayer.pause();//关闭音乐
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
  myPlayer.pause();//关闭音乐
  Mode_Stop();//熄灭所有灯
  myPlayer.play(2);//播放音乐
}
void Mode_3_rainbow()//七彩模式 单步运行
{
  if(mode_init_flag == 0){//只复位一次
    Mode_3_rainbow_Init();
    mode_init_flag = 1;
  }
  for(int i=0; i<strip.numPixels(); i++) { //生成一组彩虹色 0-360为一组彩虹，超过360的从头开始亮
    Mode_3_i = Mode_3_i + i % 360 ;        //对灯号进行处理   用Mode_3_i去计算需要显示的彩虹色，i为实际的灯号
    if(i<=120){
      Mode_3_R = 255 - Mode_3_i * 255 / 120 ;
      Mode_3_G = Mode_3_i * 255 / 120 ;
      Mode_3_B = 0 ;
    }else(i<=240){
      Mode_3_R = 0 ;
      Mode_3_G = 255 - (Mode_3_i - 120) * 255 / 120 ;
      Mode_3_B = (Mode_3_i - 120) * 255 / 120 ;
    }else(i<=360){
      Mode_3_R = (Mode_3_i - 240) * 255 / 120 ;
      Mode_3_G = 0 ;
      Mode_3_B = 255 - (Mode_3_i - 240) * 255 / 120 ;
    }
    Mode_3_R = 255 - Mode_3_R ;
    Mode_3_G = 255 - Mode_3_G ;
    Mode_3_B = 255 - Mode_3_B ;
    strip.setPixelColor(i,strip.Color(Mode_3_R,Mode_3_G,Mode_3_B));         //  Set pixel's color (in RAM)
  }    
  if(millis() - Mode_3_oldtime > 30){//这里调节彩虹循环的速度
    Mode_3_i = Mode_3_i + 1 ; //序号灯每次自加1
  }
  strip.show();             //RGB灯生效
  delay(1);                 
}
void Mode_3_rainbow_Init()//七彩模式 初始化
{
  myPlayer.pause();//关闭音乐
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
  myPlayer.pause();   //关闭音乐
  Mode_Stop();        //熄灭所有灯  
  myPlayer.play(4);   //播放音乐
}
void Mode_0_Nobody(){//无人模式 控制随机生成音乐  
//功能：无人时一直执行这个函数，每间隔15秒随机出一个新的音乐
  mp3_between_time = run_time_limit * 2 ;  //（每隔30秒随机一段15s声音）
  if(millis() - mp3_begin_time > mp3_between_time){//当前模式执行15秒以后停止
    do{
      nobody_new_mp3num = random(1,5);
    }while(nobody_new_mp3num == nobody_old_mp3num);//随机1~4音乐序号
    
    Serial.print("old_MP3_number:");   Serial.println(nobody_old_mp3num);//打印无人时，随机生成的音乐序号
    Serial.print("new_MP3_number:");   Serial.println(nobody_new_mp3num);//打印无人时，随机生成的音乐序号
    mp3_begin_time = millis();    //重置时间初始标志
  }
}
void Mode_Nobody()//无人触发模式 随机一种颜色   *注意，执行一次就会重新生成一种颜色 
{
  colorWipeAll(strip.Color(send_r,send_g,send_b));
  strip.show();
  myPlayer.pause();//当执行效果规定时间到了以后停止当前音乐,停止的是4种模式运行的音乐
}
void Mode_Stop()//所有灯带熄灭
{
  strip.clear();//灯带全黑
  strip.show();//效果生效
}

/****************************************************************/

void setup() {
  Serial.begin(115200);           //USB串口波特率
  mySerial.begin(9600);           //MP3模块串口波特率
  myPlayer.begin(mySerial);       //MP3模块初始化
  myPlayer.volume(mp3_volume);    //MP3模块音量大小

  strip.begin();                  //WS2812初始化
  strip.setBrightness(BRIGHTNES); // Set BRIGHTNESS to about (max = 255)
  strip.clear();                  //灯带全黑
  strip.show();                   //效果生效
  
  mesh.setDebugMsgTypes( ERROR | STARTUP );

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT ); //初始化mesh网络
  
  mesh.onReceive(&receivedCallback);                      //收到一条群发消息，返回消息值
  mesh.onNewConnection(&newConnectionCallback);           //mesh网络中检测到新节点，并读取nodeID值
  mesh.onChangedConnections(&changedConnectionCallback);  //mesh网络中发生变动
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);     //同步mesh时间戳

  userScheduler.addTask(taskSendMessage);   //新建子线程，可用于数据传输
  taskSendMessage.enable();                 //子线程使能
  
  pinMode(MICROWAVEPIN, INPUT);             //初始化微波传感器管脚为输入模式
  
//  16号管脚不支持中断触发 
//  pinMode(MICROWAVEPIN, INPUT_PULLUP);
//  attachInterrupt(digitalPinToInterrupt(MICROWAVEPIN), attachInterrupt_fun, FALLING);   //设置微波传感器管脚为中断下降沿触发

  /* 灯带基础显示程序
  colorWipe(strip.Color(  0,   0, 255), 5); // Blue
  colorWipeAll(strip.Color(  0,   0, 255));
  rainbow(20);
  rainbowCycle(20); */
}

void loop(){
  mesh.update();    //尽可能让这句话频繁运行
  
  if(digitalRead(MICROWAVEPIN)==0 && run_flag == 0){  //无效果运行时，微波传感器被触发 
    attachInterrupt_fun();  //触发处理程序
    run_flag = 1 ;          // 开始执行效果标志，0：无人   1：有人触发
  }
  
  if(run_flag==1){ //开始运行标志(记录效果开始时的系统运行时间)
    begin_time = millis();        //记录当前的开始时间
    run_flag = 2;                 //2代表开始执行动态效果
  }
  
  if(run_flag==2){ //自加计数程序(在运行过程中记录程序循环的次数)
    run_number = run_number + 1 ; //标志模式运行次数，每次loop自加1
  }
  
  if((millis() - begin_time > run_time_limit) && run_flag == 2 ){//当前模式执行15秒以后停止
    run_flag = 0;         // 无人运行标志 使用系统运行时间来结束当前效果
    old_mode = run_mode;  // 记录上次的模式
    run_mode = 0;         // 重新设置为无人运行模式
    mode_init_flag = 0;   // 模式复位标志位
    Mode_Nobody();        // 无人时显示一种随机颜色 （执行一次，生成个颜色就完事）
    nobody_old_mp3num = random(1,5);//
    myPlayer.play(nobody_old_mp3num);//播放随机音乐
    Serial.println("MICROWAVE PIN READY!!!");//打印接受管脚状态
  }
  
  switch (run_mode)
  {
    case 0:   Mode_0_Nobody();       break;//一直执行，累计时间，控制无人时的声音
    case 1:   Mode_1_Morning();      break;//每执行一次效果运动一点
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
  
  if(run_flag == 0){  //从无人状态切换到动态效果时，做一次运行次数归零
    run_number = 0;   //程序运行次数清零
    do{
      run_mode = random(1,5);
    }while(run_mode == old_mode);//随机出新的mode，并群发给其他云
    do{
      send_r = random(0,255);
      send_g = random(0,255);
      send_b = random(0,255);
    }while(!(send_r >= 200 || send_g >= 200 || send_b >= 200));//如果随机出暗色，就再重新取一次值
  }
  Serial.print("R:");   Serial.print(send_r);   Serial.print("  ");//查看生成的 R G B M 值
  Serial.print("G:");   Serial.print(send_g);   Serial.print("  ");
  Serial.print("B:");   Serial.print(send_b);   Serial.print("  ");
  Serial.print("M:");  Serial.print(run_mode);  Serial.println("  ");
}
