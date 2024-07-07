#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h> // Include DHT library

// WiFi settings
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT settings
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_user = "";  
const char* mqtt_password = "";   
const char* sensorDataTopic = "sensor/data"; // MQTT topic for sensor data

const int DHT_PIN = 23; // DHT22 sensor pin

WiFiClient espClient;
PubSubClient client(espClient);

DHTesp dht; // Initialize DHT sensor object

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  
  dht.setup(DHT_PIN, DHTesp::DHT22); // Initialize DHT sensor
  
  // Wait a few seconds to stabilize sensor readings
  delay(2000);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  // Read data from DHT sensor
  TempAndHumidity data = dht.getTempAndHumidity();
  
  if (isnan(data.temperature) || isnan(data.humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }

  // Format data as JSON
  String jsonString = "{\"temperature\":" + String(data.temperature, 2) + ",\"humidity\":" + String(data.humidity, 1) + "}";
  char jsonCharArray[jsonString.length() + 1];
  jsonString.toCharArray(jsonCharArray, jsonString.length() + 1);

  // Publish data to MQTT broker
  client.publish(sensorDataTopic, jsonCharArray);

  // Print to serial monitor for debugging
  Serial.println("Published data:");
  Serial.println(jsonString);
  Serial.println("---");

  client.loop(); // Ensure MQTT client is connected and processes incoming messages

  delay(5000); // Wait 5 seconds before next reading
}
