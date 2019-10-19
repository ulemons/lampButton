#include "WiFi.h"
#include "yeelight.h"
#include <ArduinoJson.h>

const char* ssid = "***";
const char* password =  "***";


const int button = 12;
const int in = 13;

StaticJsonBuffer<200> jsonBuffer;
Yeelight* yeelight;
int counter = 0;


void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  yeelight = new Yeelight();
  yeelight->lookup();
  Serial.println("Connected to the WiFi network");
  pinMode(button, OUTPUT);
  pinMode(in, INPUT);

}

void loop() {
  int Push_button_state = digitalRead(in);
  Serial.println(Push_button_state);
  if ( Push_button_state == 0) {
    counter++;
    if (counter == 5){
      Serial.println("Accendi la luce"); 
      counter = 0;
      Serial.println("feedback");
      Serial.println(yeelight->feedback());
      if (yeelight->feedback()) {
      Serial.print("device: ");
      Serial.println(yeelight->getLocation());
      JsonObject& root = jsonBuffer.parseObject(yeelight->sendCommand("get_prop", "[\"power\", \"name\"]"));
      const char* state = root["result"][0];
      const char* name = root["result"][1];
      const char stanza[] = "stanza";
      Serial.print("- power is: ");
      Serial.println(state);
      Serial.print("- name is: ");
      Serial.println(name);
      Serial.print("strcmp: ");
      Serial.println(strcmp(name,"stanza"));
      if (!strcmp(name,"stanza")){

//      Serial.println(yeelight->sendCommand("set_name", "[\"stanza\"]"));
        
      if (!yeelight->isPowered()) {
        Serial.println(yeelight->sendCommand("set_power", "[\"on\", \"smooth\", 250]"));
      } else {
        Serial.println(yeelight->sendCommand("set_power", "[\"off\", \"smooth\", 250]"));
        
        }
      }
      yeelight = new Yeelight();
      yeelight->lookup();
      
      } else {
        Serial.println("Impossibile accendere la luce");
      }  
      delay(2000);
     }
    
  } else {
    counter = 0;
  }
}
