#include <PubSubClient.h>
#include <NTPClient.h>
#include <time.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "jsn-sr04t.h"
#include "webserver.h"

#define STASSID "Dminty"
#define STAPSK  "000"

#define FW_VER "1.25"

ADC_MODE(ADC_VCC);

const char *ssid = STASSID;
const char *password = STAPSK;
const char* host = "ultrasonic";

const char* mqtt_host = "192.168.2.63";
int mqtt_port = 1883;

const char* mqtt_login = "dacha";

#define WEB_PAGE_SIZE 2048

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient espClient;
PubSubClient mqtt(espClient);
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP);

#define DISTANCE_READ_INTERVAL 5000
#define MQTT_SEND_INTERVAL 60*1000
#define NTP_UPDATE_INTERVAL 1*60*1000

os_timer_t mqtt_timer;
os_timer_t distance_timer;
os_timer_t ntp_timer;

int distance = 0;

JsnSr04t jsnSr04t;

#define LOCALTIME2STR "%a, %d.%m.%Y %X"  //http://all-ht.ru/inf/prog/c/func/strftime.html
#define LOCALTIMESTRLENMAX 30

void get_localtime(char* buf){

    unsigned long epochTime = ntpClient.getEpochTime();
    struct tm *timeinfo = gmtime ((time_t *)&epochTime); 


    strftime(buf, LOCALTIMESTRLENMAX, LOCALTIME2STR, timeinfo);
/*
    snprintf(buf, LOCALTIMESTRLENMAX, LOCALTIME2STR,  
                                                        timeinfo.tm_wday,
                                                        timeinfo.tm_mday,
                                                        timeinfo.tm_mon,
                                                        timeinfo.tm_year,
                                                        timeinfo.tm_hour,
                                                        timeinfo.tm_min,
                                                        timeinfo.tm_sec);
*/
    //return buf;
}
    
void handleRoot() {
  char temp[WEB_PAGE_SIZE];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  char hostname[20] = "esp: ";
  strcat(hostname, host);

  sprintf(temp, PAGE_HEADER, hostname, STYLE_CSS);
  sprintf(temp + strlen(temp), "<h1>Host: %s</h1>", host);
  sprintf(temp + strlen(temp), "<h4>FW version: %s</h4>", FW_VER);
  sprintf(temp + strlen(temp), "<p>Freemem: %d</p>", ESP.getFreeHeap());
  sprintf(temp + strlen(temp), "<p>VCC: %d</p>", ESP.getVcc());


  char * buf2 = (char *)malloc(25);
  get_localtime(buf2);
  //sprintf(temp + strlen(temp), "<p>Time: %s</p>", ntpClient.getFormattedTime().c_str());
  sprintf(temp + strlen(temp), "<p>Time: %s</p>", buf2);
  free(buf2);
  
  sprintf(temp + strlen(temp), "<p>Uptime: %02d:%02d:%02d</p>", hr, min % 60, sec % 60);
  sprintf(temp + strlen(temp), "<p>Distance: %d.%d cm</p>", (int)distance / 10, (int)distance % 10);
  char footer[100] = "<span><a href='/update'>OTA</a></span>";
  sprintf(temp + strlen(temp), PAGE_FOOTER, footer);  
  server.send(200, "text/html", temp);
}

void handleOTA(){
  char temp[400];
snprintf(temp, 400,
  
    "<html>\
      <head>\
      <title>ESP8266 Demo</title>\
      <style>\
        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      </style>\
    </head>\
    <body>\
      <h1>OTA Update!</h1>\
      <h2>Hostname: %s</h2>\
      <h4>FW version: %s</h4>\
    </body>\
  </html>",
              host,
              FW_VER
            );  
  server.send(200, "text/html", temp);
 
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message); 
}


void wifiConnect() {
   WiFi.disconnect();
   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);

    // Wait for connection
    static int con_cnt = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());     
}

void handleMqttData(char* topic, byte* payload, unsigned int length) {
  Serial.printf("Data received, topic: %s, data: %s\r\n", topic, payload);
}

void mqtt_publish_data_cb(){
       char topic[30];
      strcpy(topic, mqtt_login);
      strcat(topic, "/");
      strcat(topic, host);
      strcat(topic, "/");
      strcat(topic, "distance");

      char payload[10];
      sprintf(payload, "%d.%d", (int)distance / 10, (int)distance % 10);
      Serial.printf("topic: %s, data: %s\n", topic, payload);
      mqtt.publish(topic, payload);   
}

void distance_read_cb(){
      distance = jsnSr04t.getDistance();
      //char str[10] = "";
      //sprintf(str, "Distance: %d.%d cm", (int)distance / 10, (int)distance % 10);
      //Serial.println(str);  
}

void ntp_update_cb(){
  Serial.println("Get NTP update..."); 
  if ( ntpClient.update() ) {
      Serial.printf("NTP Time: %s\n\r", ntpClient.getFormattedTime().c_str());
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);

  wifiConnect();

  httpUpdater.setup(&server);
  
  server.on("/", handleRoot);
  server.on("/ota", handleOTA);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

       //Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
  jsnSr04t.begin(2, 0);

  //topic, data, data is continuing
  mqtt.setServer(mqtt_host, mqtt_port);
  mqtt.setCallback(handleMqttData);

  ntpClient.begin();
  ntpClient.setTimeOffset(3600*3);
  ntp_update_cb();
  
  os_timer_disarm(&mqtt_timer);
  os_timer_setfn(&mqtt_timer, (os_timer_func_t *)mqtt_publish_data_cb, NULL);
  os_timer_arm(&mqtt_timer, MQTT_SEND_INTERVAL, 1);

  os_timer_disarm(&distance_timer);
  os_timer_setfn(&distance_timer, (os_timer_func_t *)distance_read_cb, NULL);
  os_timer_arm(&distance_timer, DISTANCE_READ_INTERVAL, 1);    

  os_timer_disarm(&ntp_timer);
  os_timer_setfn(&ntp_timer, (os_timer_func_t *)ntp_update_cb, NULL);
  os_timer_arm(&ntp_timer, NTP_UPDATE_INTERVAL, 1);   
}

void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    //Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (mqtt.connect(host)) {
      //Serial.println("connected");
      // Once connected, publish an announcement...

      // ... and resubscribe
      //mqtt.subscribe("inTopic");
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(mqtt.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

    if (WiFi.status() != WL_CONNECTED)
    {
      wifiConnect();
    }  
    
    server.handleClient();

  if (!mqtt.connected()) {
    mqtt_reconnect();
  }
  mqtt.loop();
    
}
