#include "painlessMesh.h"
//2021.3.2
#define   MESH_PREFIX     "clouds"
#define   MESH_PASSWORD   "sattvalab"
#define   MESH_PORT       5555
//TASK_SECOND = 1000   //count
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage ); 
//创建一个子任务 间隔为一秒执行一次（初始化发送时间间隔）

void sendMessage() { //发送一条字符串
  String msg = "Hello from node ";
  msg += mesh.getNodeId();//字符串末尾添加信息来源
  //msg = msg + " Humidity: " + String(dht.readHumidity());//发送湿度示例
  mesh.sendBroadcast( msg );//字符串内容
  //taskSendMessage.setInterval(200);//重新设置发送间隔，单位ms
  //taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));//间隔1-5s
}

// Needed for painless library
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

void setup() {
  Serial.begin(115200);//
  Serial.print("TASK_SECOND:");
  Serial.println(TASK_SECOND);
//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );//初始化mesh网络
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );//新建子线程，可用于数据传输
  taskSendMessage.enable();//子线程使能
}

void loop() { 
  mesh.update();//尽可能让这句话频繁运行
  //delay(1000);
}
