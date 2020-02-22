#include <ArduinoJson.h>

#include <WiFiUdp.h>
#include "ESP8266WiFi.h"

const char* ssid = "****";
const char* password =  "****";


const int button = D3 ;
const int in = D4;

StaticJsonBuffer<200> jsonBuffer;

int counter = 0;

WiFiUDP _udp;
WiFiClient _client;
IPAddress _ipMulti(239, 255, 255, 250);
char _packetBuffer[550];

String _location, _support, _name;
bool _powered;
char _server[15];

uint16_t _cmdid = 0, _port = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.printf("Connection status: %d\n", WiFi.status());
    WiFi.printDiag(Serial);

    Serial.println("Connecting to WiFi..");
  }


//  Serial.println(feedback());
//  delay(1000);
  lookup();
//  Serial.println(feedback());

  Serial.println("Connected to the WiFi network");
  pinMode(button, OUTPUT);
  pinMode(in, INPUT);

}

void loop() {
  int Push_button_state = digitalRead(in);
  if ( Push_button_state == 0) {
    
      Serial.println("Accendi la luce"); 
      Serial.println("feedback");

      if (feedback()){
        Serial.print("device: ");
        Serial.println(getLocation());

    if (!isPowered()) {
      Serial.println(sendCommand("set_power", "[\"on\", \"smooth\", 150]"));
      Serial.println("command on");
    } else {
      Serial.println(sendCommand("set_power", "[\"off\", \"smooth\", 150]"));
      Serial.println("command off");
      }

    JsonObject& root = jsonBuffer.parseObject(sendCommand("get_prop", "[\"power\", \"name\"]"));
    const char* state = root["result"][0];
    const char* name = root["result"][1];
    Serial.print("- power is: ");
    Serial.println(isPowered());
    Serial.print("- name is: ");
    Serial.println(getName());
        }
              lookup();

  
      delay(200);
  }
}

void lookup()
{
  _udp.beginMulticast(WiFi.localIP(), _ipMulti, 1982);
  _udp.beginPacketMulticast(_ipMulti, 1982, WiFi.localIP());
  _udp.print("M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1982\r\nMAN: \"ssdp:discover\"\r\nST: wifi_bulb");
//   _udp.print("M-SEARCH * HTTP/1.1\r\nMAN: \"ssdp:discover\"\r\nST: wifi_bulb");
_udp.endPacket();
  _udp.begin(1982);
}


int feedback()
{
  int packetSize = _udp.parsePacket();
  Serial.println(packetSize);
  if (packetSize) {
    int len = _udp.read(_packetBuffer, 550);
    if (len > 0) {
      _packetBuffer[len] = 0;
    }
    parseFeedback(_packetBuffer, len);
  }
  return packetSize;
}


void parseFeedback(char* buffer, size_t len)
{
  Serial.println(buffer);
  int i=0, _i=0;
  char _b[255];
  while (i<len) {
    if (buffer[i] == '\r' &&
        i+1 <= len &&
        buffer[i+1] == '\n') {
        _b[_i]=0;

        // ----
        String _str = String(_b);
        if (_str.startsWith("Location: yeelight://")) {
          int colon = _str.indexOf(':', 21) + 1;
          _location = _str.substring(10);
          _str.substring(21, colon).toCharArray(_server, colon - 21);
          _port = _str.substring(colon).toInt();
        }
        if (_str.startsWith("support: ")) {
          _support = _str.substring(9);
        }
        if (_str.startsWith("power: ")) {
          _powered = _str.substring(7) == "on";
        }
        if (_str.startsWith("name: ")) {
          _name =  _str.substring(6);
        }
        // ----

        i=i+2;
        _i=0;
    } else {
      _b[_i]=buffer[i];
      i++;
      _i++;
    }
  }
}

String sendCommand(String method, String params)
{
  if (_client.connect(_server, _port)) {
    String payload = String("") + "{\"id\":" + (++_cmdid) + ",\"method\":\"" + method + "\",\"params\":" + params + "}";
    _client.println(payload);
  }

  String result = "";
  while (_client.connected()) {
    result = _client.readStringUntil('\r');
    _client.stop();
  }
  return result;
}

bool isPowered()
{
  return _powered;
}

String getSupport()
{
  return _support;
}

String getLocation()
{
  return _location;
}

String getName()
{
   return _name;
  }
