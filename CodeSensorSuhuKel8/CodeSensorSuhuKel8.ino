#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// --- WI-FI SETTINGS ---
const char* ssid = "MAWA";        // CHANGE THIS
const char* password = "Murahmeriah15"; // CHANGE THIS

// --- MQTT BROKER SETTINGS ---
const char* mqtt_server = "160.187.144.142";
const int mqtt_port = 1883;
const char* topic = "student/sensor/Abimanyu3TIA"; // Your unique topic

// --- HARDWARE SETTINGS ---
#define SENSOR_PIN 2  // GPIO2 (D4 on board)
#define LED_PIN    2  // GPIO2 (Same pin as sensor on Lolin/Wemos)
#define DHTTYPE    DHT11

DHT dht(SENSOR_PIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Ensure LED is OFF at start
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // While waiting for Wi-Fi, blink slowly
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, LOW); // ON
    delay(250);
    digitalWrite(LED_PIN, HIGH); // OFF
    delay(250);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Create a random client ID so the server doesn't kick us off
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      // --- ERROR: BLINK REPEATEDLY ---
      // Instead of just waiting, we strobe the light to show error
      for(int i = 0; i < 25; i++) { // Blink fast for 5 seconds
        digitalWrite(LED_PIN, LOW);  // ON
        delay(100);
        digitalWrite(LED_PIN, HIGH); // OFF
        delay(100);
      }
    }
  }
}

void loop() {
  // 1. Check Connection
  if (!client.connected()) {
    reconnect(); // If disconnected, this will start the "Error Blinking"
  }
  client.loop();

  // 2. Read Sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // 3. Publish Data
  if (!isnan(h) && !isnan(t)) {
    String msg = "{\"temp\": " + String(t) + ", \"hum\": " + String(h) + "}";
    
    Serial.print("Publishing: ");
    Serial.println(msg);
    
    client.publish(topic, msg.c_str());
    
    // --- SUCCESS: BLINK TWICE ---
    // Blink 1
    digitalWrite(LED_PIN, LOW);  // ON
    delay(150);
    digitalWrite(LED_PIN, HIGH); // OFF
    delay(150);
    
    // Blink 2
    digitalWrite(LED_PIN, LOW);  // ON
    delay(150);
    digitalWrite(LED_PIN, HIGH); // OFF
  }

  // 4. Wait 2 Seconds before next reading
  delay(2000);
}