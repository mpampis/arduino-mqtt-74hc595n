#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Wire.h>
#define number_of_74hc595s 5
#define numOfRegisterPins number_of_74hc595s * 8
const int SER_Pin = 4;   //pin 14 on the 75HC595
const int RCLK_Pin = 3;  //pin 12 on the 75HC595
const int SRCLK_Pin = 2; //pin 11 on the 75HC595
int value = 0;
int switchNo = 0;
int switchStatus = 0;
boolean registers[numOfRegisterPins];
byte mac[] = {   0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
IPAddress ip(192, 168, 2, 205);
IPAddress server(192, 168, 2, 11);
EthernetClient ethClient;
PubSubClient client(ethClient);

void setup()
{
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
 Serial.begin(9600);
  client.setServer(server, 1883);
  client.setCallback(callback);
  Ethernet.begin(mac, ip);
  delay(1500);
}
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
void clearRegisters() {
  for (int i = numOfRegisterPins - 1; i >=  0; i--) {
    registers[i] = LOW;
  }
}
void writeRegisters() {
  digitalWrite(RCLK_Pin, LOW);
  for (int i = numOfRegisterPins - 1; i >=  0; i--) {
    digitalWrite(SRCLK_Pin, LOW);
    int val = registers[i];
    digitalWrite(SER_Pin, val);
    digitalWrite(SRCLK_Pin, HIGH);
  }
  digitalWrite(RCLK_Pin, HIGH);
}
//set an individual pin HIGH or LOW
void setRegisterPin(int index, int value) {
  registers[index] = value;
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
 //   Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduino")) {
//      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("Stat/Arduino1/LWT/", "Online");
      // ... and resubscribe
      client.subscribe("/Home/Center1/");
    }
    else {
      delay(5000); // Wait 5 seconds before retrying
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {

  switchNo = (payload[0] & 0xf) * 10 + (payload[1] & 0xf);
  switchStatus = (payload[2] & 0xf);
  setRegisterPin(switchNo, switchStatus);
  writeRegisters();

 const char *p = reinterpret_cast<const char*>(payload);
char sub[3];
strncpy(sub, p, 2);
sub[2] = '\0';
//openhab item update 
  if (switchStatus == 1) {
    client.publish(sub, "ON");
  }
  else if (switchStatus == 0) {
    client.publish(sub , "OFF");
  }
}


