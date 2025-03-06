/*
length of 10mL syringe is ~5cm, this means ~80000 steps.
1 step is 0.000125mL=0.125uL=125nL
note that in program, steps must be multipled by 2

*/

#include <credentials.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "ArduinoJson.h"

const String version = "0.2";
const char* deviceName = "pump_3";
const char* deviceType = "pump";
const char* mqtt_server = "10.0.0.4";
WiFiClient esp_pump_3;
PubSubClient client(esp_pump_3);
const unsigned long long_max = 4294967295;


//comment out unnecessary
//#define esp32_wrover 0
#define xiao32_c3 0

#ifdef esp32_wrover
  const byte pin_1_s = 12; //step pin
  const byte pin_1_d = 14; //dir pin
  const byte pin_1_e = 27; //endstop pin
  const byte pin_1_com = 32; //pin for communication between boards
#endif
#ifdef xiao32_c3
const byte pin_1_s = 7; //step pin, d5 on board
const byte pin_1_d = 21; //dir pin, d6 on board
const byte pin_1_e = 6; //endstop pin, d4 on board
const byte pin_1_com = 5; //pin for communication between boards, d3 on board
#endif

int speed_1 = 70; //70 is minimum
int block_num = 0;
const int block_size = 10;
int phase_1 = 0;
unsigned long timer_1 = 0;
unsigned long next_step_1 = 0; //when stepper moves
unsigned long block_start = 0; //when program block started
unsigned long block_stop = 0;
long absolute_pos_1 = 0;
long requested_pos_1 = 0;
int direction_1 = 1;
int program_length = 0;
JsonDocument program_1; //program buffer ["step1":["delay":70,"micros":1000],"step2":...]
JsonDocument program_0;//executed program
bool run_program_switch = 0;
/*=====================
function declarations for connectivity
=====================*/


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(mySSID);
  //WiFi.setSleepMode(WIFI_NONE_SLEEP); //should prevent random disconnects > https://github.com/esp8266/Arduino/issues/5083
  WiFi.mode(WIFI_STA); //WiFi mode station (connect to wifi router only
  WiFi.begin(mySSID, myPASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String ttopic = String(topic);
  byte p[length];
  for (int i=0;i<length;i++) {
    p[i] = payload[i];
  }
  String pp= (char*)p;
  pp = pp.substring(0,length); //w/o this some weird data is added to stirng
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print("; length: ");
  Serial.print(length);
  Serial.print("; payload: ");
  Serial.println(pp);

  if (ttopic==deviceName+String("/run_master") || ttopic==deviceName+String("/run_slave")){
    
    program_length = program_0.size();
    Serial.print("program_length:");
    Serial.println(program_length);
    speed_1 = int(program_0[0][0]);
    for (int i = 0; i<program_length;i++){
      Serial.println(String(program_0[i][1]));
    }
    
    if (speed_1 <0){
      direction_1 = 1;
      digitalWrite(pin_1_d,1);
    }
    else{
      direction_1 = -1;
      digitalWrite(pin_1_d,0);
    }
    speed_1 = abs(speed_1);
    run_program_switch = 1;
    block_num = 1;
    //Serial.println("pump will run program");
    if (ttopic==deviceName+String("/run_slave")){
      pinMode(pin_1_com,INPUT);
      while (digitalRead(pin_1_com)==0){;}
    }
    else{
      pinMode(pin_1_com,OUTPUT);
      delay(1000); //makes sure that other devices are ready
      digitalWrite(pin_1_com,1); //starts "on" signal
    }
    next_step_1 = micros();
    block_start = next_step_1;
    block_stop = block_start + int(program_0[1][1]);
    if(speed_1 == 0){
      next_step_1 = block_stop;
    }
    //Serial.println(next_step_1);
    //Serial.println(block_stop);
    //Serial.println(block_stop-next_step_1);
  }/*
  else if (ttopic==deviceName+String("/run_slave")){
    pinMode(pin_1_com,INPUT);
    speed_1 = int(program_0[0][0]);
    if (speed_1 <0){
      direction_1 = 1;
      digitalWrite(pin_1_d,1);
    }
    else{
      direction_1 = -1;
      digitalWrite(pin_1_d,0);
    }
    speed_1 = abs(speed_1);
    run_program_switch = 1;
    block_num = 1;
    //Serial.println("pump will run program");
    while (digitalRead(pin_1_com)==0){;}
    next_step_1 = micros();
    block_start = next_step_1;
    block_stop = block_start + int(program_0[1][1]);
  }*/
  else if (ttopic==deviceName+String("/new_program")){
    deserializeJson(program_0, pp); //Parse message
    //Serial.print("program received: ");
    //Serial.println(int(program_1[0][0]));
  }
  else if (ttopic==deviceName+String("/continue_program")){
    deserializeJson(program_1, pp); //Parse message
    for (int i =0; i<program_1.size();i++){
      program_0.add(program_1[i]); 
    }
    //Serial.print("program received: ");
    //Serial.println(int(program_1[0][0]));
  }
  else if (ttopic==deviceName+String("/homing")){
    homing();
  }
  else if (ttopic==String("common/device_scan")){
    //client.publish("common/device_response", (String("'MQTTname':'") + deviceName + String("','type':'")+ deviceType + String("'")).c_str());
    client.publish("common/device_response", (deviceName + String(":")+ deviceType).c_str());
  }



}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(deviceName)) {
      Serial.println("connected");
      // Convert String to const char* using c_str()
      client.publish("common/status", (deviceName + String(" connected")).c_str());
      char a[16];
      itoa(absolute_pos_1, a, 10);
      client.publish((deviceName + String("/position_now")).c_str(), a);
      // Subscribe to topics
      client.subscribe((deviceName + String("/speed")).c_str());
      client.subscribe((deviceName + String("/steps")).c_str());
      client.subscribe((deviceName + String("/end_position")).c_str());
      client.subscribe((deviceName + String("/run")).c_str());
      client.subscribe((deviceName + String("/run_slave")).c_str());
      client.subscribe((deviceName + String("/run_master")).c_str());
      client.subscribe((deviceName + String("/continue_program")).c_str());
      client.subscribe((deviceName + String("/new_program")).c_str());
      client.subscribe((deviceName + String("/homing")).c_str());
      client.subscribe(String("common/device_scan").c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*=====================
function declarations for pumping
=====================*/

void pump_service(){
  unsigned long tn = micros();
  if(run_program_switch == 1){ //checks if it is program run
    if(tn <= block_stop){ //checks if block isn't done
      if(tn>=next_step_1){ //checks if next step now
        //Serial.println("single step");
        phase_1 = abs(phase_1-1); //single step
        digitalWrite(pin_1_s,phase_1);
        next_step_1 += speed_1;
        absolute_pos_1 += direction_1;
      } //end of this step
    }//end of present block
    else{ //if block is done
      if (block_num >= program_length){//checks if it was last block
        next_step_1 = long_max;
        run_program_switch = 0;
        char cstr[16];
        itoa(absolute_pos_1/2, cstr, 10);        
        digitalWrite(pin_1_com,0); //turns off start signal
        client.publish((deviceName + String("/position_now")).c_str(), cstr);
      }
      else{
        block_start = tn;
        block_stop = block_start + int(program_0[block_num+1][1]);
        speed_1 = int(program_0[block_num][0]);
        Serial.print("block_stop: ");
        Serial.print(block_stop);
        Serial.print("; speed: ");
        Serial.println(speed_1);
        if (speed_1 <0){//sets direction
          direction_1 = 1;
          digitalWrite(pin_1_d,1);
        }
        else if(speed_1 == 0){
          next_step_1 = block_stop;
        }
        else{
          direction_1 = -1;
          digitalWrite(pin_1_d,0);
        }
        speed_1 = abs(speed_1);
        block_num++;
      }
    }
  } //end of program run
    
  else if(tn>=next_step_1){
    if(absolute_pos_1 == requested_pos_1){ 
      //if goal reached, write timing info and sets next step to distant number
      char cstr[16];
      itoa(absolute_pos_1/2, cstr, 10);
      client.publish((deviceName + String("/position_now")).c_str(), cstr);
      //if (series_length >0){
      //  series_length--;
      //  while (digitalRead(pin_1_com)==1){;}
      //  client.publish("pump_master/step_done","Y");
      //}
      next_step_1 = long_max;}
    else{
      phase_1 = abs(phase_1-1);
      digitalWrite(pin_1_s,phase_1);
      next_step_1 += speed_1;
      absolute_pos_1 += direction_1;
      
    }
  }
}
//client.publish("pump_master/step_done","Y");
void homing(){
  digitalWrite(pin_1_d,1);
  for(int i=0; i<6000;i++){ //initial retract, just in case
    digitalWrite(pin_1_s,1);
    delayMicroseconds(70);
    digitalWrite(pin_1_s,0);
    delayMicroseconds(70);
  }
  digitalWrite(pin_1_d,0);
  while(digitalRead(pin_1_e)!=0){ //crude homing
    digitalWrite(pin_1_s,1);
    delayMicroseconds(70);
    digitalWrite(pin_1_s,0);
    delayMicroseconds(70);
  }
  digitalWrite(pin_1_d,1);
  for(int i=0; i<1000;i++){ //retract
    digitalWrite(pin_1_s,1);
    delayMicroseconds(70);
    digitalWrite(pin_1_s,0);
    delayMicroseconds(70);
  }
  digitalWrite(pin_1_d,0);
  while(digitalRead(pin_1_e)!=0){ //precise homing run
    digitalWrite(pin_1_s,1);
    delayMicroseconds(500);
    digitalWrite(pin_1_s,0);
    delayMicroseconds(500);
  }
  absolute_pos_1 = 0;
  client.publish((deviceName + String("/position_now")).c_str(), "0");
}

/*=====================
program executions
=====================*/

void setup(){
  pinMode(pin_1_s, OUTPUT);
  pinMode(pin_1_d, OUTPUT);
  pinMode(pin_1_e, INPUT_PULLUP);
  pinMode(pin_1_com, OUTPUT);
  delay(200);
  Serial.begin(2000000);
  Serial.print("this is infusion pump v");
  Serial.println(version);
  setup_wifi();
  Serial.println("really!");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  timer_1=micros();
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  pump_service();
}
