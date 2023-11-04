#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h> // Include the DHT library
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// BLE server name
#define bleServerName "DHT11_ESP32"

// Define the DHT sensor pin and type
#define DHTPIN 4    // GPIO2 for example
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);

float temp;
float hum;
float lux;

#include <BH1750.h>
BH1750 lightMeter;

const char* ssid = "";
const char* password = "";

const char* mqtt_server = "145.24.222.116";
const char* mqtt_user = "minor";
const char* mqtt_pass = "smartthings2023";
const char* mqtt_topic = "weather";

WiFiClient wifi_client;
PubSubClient client(wifi_client);

void reconnect();

bool mqttConnect(){
  client.setServer(mqtt_server, 8884);
  Serial.println("Connecting to MQTT broker");
  while(!client.connected()){
    if(client.connect("esp32", mqtt_user, mqtt_pass)){
      Serial.println("CONNECTED TO MQTT");
    }
    Serial.print(".");
  }
  Serial.println();
  return true;
}

void initWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println(WiFi.localIP());
  mqttConnect();
}

void setup() {
  // Start serial communication 
  Serial.begin(115200);
  // Initialize the DHT sensor
  dht.begin();
  Wire.begin();
   // Initialize the light sensor
  lightMeter.begin();
  // Initialize WIFI
  initWiFi();
}

void loop() {

   // Read temperature and humidity from DHT sensor
   temp = dht.readTemperature();
   hum = dht.readHumidity();
   lux = lightMeter.readLightLevel();
   
  if(WiFi.status() == WL_CONNECTED){
    if(client.connected()){
       String data = "{\"user\":\"KasperOfzeau\", \"weather_station\": 4, \"data\": {\"temperature\":\""+ (String)temp+"\",\"humidity\":\""+ (String)hum+"\",\"light_intensity\":\""+ (String)lux+"\"}}";
      if(!isnan(temp) && !isnan(hum) && !isnan(lux)){
        Serial.print("Sent: ");
        Serial.println(data);
        const char* d = data.c_str();
        client.publish(mqtt_topic, d);
      }
    }else{
      mqttConnect();
    }
  }else{
    initWiFi();
  }
  client.loop();
  delay(600000);
}
