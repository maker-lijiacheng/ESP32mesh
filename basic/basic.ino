#include "painlessMesh.h"
//2021.3.2
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
//TASK_SECOND = 1000   //count
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
//创建一个子任务 间隔为一秒执行一次

void sendMessage() { //发送一条字符串
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval(200);//间隔1-5s
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

  userScheduler.addTask( taskSendMessage );//创立子线程，可用于数据传输
  taskSendMessage.enable();//子线程使能
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();//尽可能让这句话频繁运行




  
}
