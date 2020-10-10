/*
MQTT Client for werkplaats TkkrLab
Author : Dave Borghuis

*/

#include <M5StickC.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "config.h"

//WIFI
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;
const char* mqtt_server = MQTT_SERVER;

//MQTT client
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

bool StateWerkplaats = false;
bool StateSpace = false;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  if (strcmp(topic,"tkkrlab/werkplaats/state") == 0) {

    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
      Serial.println("Werkplaats Turn space open.");
      StateWerkplaats = true;
    } else {
      Serial.println("Werkplaats Turn space closed.");  
      StateWerkplaats = false;
    }
  };

  if ( strcmp(topic,"tkkrlab/spacestate") == 0) {
      // Switch on the LED if an 1 was received as first character
      if ((char)payload[0] == '1') {
        Serial.println("Space Turn space open.");
        StateSpace = true;
      } else {
        Serial.println("Space Turn space closed.");    
        StateSpace = false;
      }
  }
  
  displayStatus();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {

    Serial.print("Attempting MQTT connection...");
    String clientId = "MQTT_Werkplaats";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("tkkrlab/werkplaats/state");
      client.subscribe("tkkrlab/spacestate");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {

  M5.begin();
  M5.Lcd.setRotation(1);
  
  M5.Lcd.fillScreen(BLUE);
  M5.Lcd.setCursor(15, 10);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(5);
  M5.Lcd.printf("BOOT");
    
  Serial.begin(115200);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(BUTTON_A_PIN, INPUT);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(digitalRead(BUTTON_A_PIN) == 0) {
    client.publish("tkkrlab/werkplaats/io/input/0", "1");
    Serial.print("Button Werkplaats !!");   
    delay(500); 
  };

}

void displayStatus() {
  if (StateWerkplaats) {
    // Werkplaats open 
    M5.Lcd.fillScreen(GREEN); //GREEN
    M5.Lcd.setCursor(18, 10);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(5);
    M5.Lcd.printf("OPEN");
  } else {
    // werkplaats gesloten
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setCursor(7, 14);
    M5.Lcd.setTextColor(0xFFE0); //yellow
    M5.Lcd.setTextSize(4);
    M5.Lcd.printf("CLOSED");     
  }

  if (StateSpace) {
    M5.Lcd.fillRect(0, 60, 160, 30, GREEN);
    M5.Lcd.setCursor(20, 63);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.printf("SPACE OPEN");
  } else {
    M5.Lcd.fillRect(0, 60, 160, 30, RED);
    M5.Lcd.setCursor(10, 63);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(0xFFE0); 
    M5.Lcd.printf("SPACE CLOSED");     
  }
    
}
