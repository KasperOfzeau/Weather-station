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
const char* api_server = "http://145.24.222.116:8000/dashboard/weather/";

WiFiClient wifi_client;
// WiFiClientSecure wifi_client;
PubSubClient client(wifi_client);

long last_msg = 0;
char msg[50];
int value = 0;

void callback(char* topic, byte* message, unsigned int length);
void reconnect();

bool mqttConnect(){
  client.setServer(mqtt_server, 8884);
  // Serial.println(CA_KEY);
  Serial.print("Connecting to MQTT broker");
  while(!client.connected()){
    if(client.connect("esp32", mqtt_user, mqtt_pass)){
      Serial.println("CONNECTED TO MQTT");
    }else{
      Serial.print("Failed with state ");
      char err_buf[100];
      Serial.println(client.state());
      delay(2000);
    }
    Serial.print(".");
  }
  Serial.println();
  client.subscribe("incoming");
  return true;
}

void initWiFi(){
  WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password, 11, bssid, true);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println(WiFi.localIP());
  // wifi_client.setCACert(CA_KEY);
  mqttConnect();
 }

void setup() {
  // Start serial communication 
  Serial.begin(115200);

  // Initialize the DHT sensor
  dht.begin();

  Wire.begin();
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
      if(!isnan(temp) && !isnan(hum)){
        Serial.print("Sent: ");
        Serial.println(data);
        const char* d = data.c_str();
        client.publish("test", d);
      }
    }else{
      mqttConnect();
    }
  }else{
    initWiFi();
  }
  client.loop();
  delay(10000);
}
