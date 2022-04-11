#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Config.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <WiFi.h>

/***** DEFINES *****/
#define MEASUREMENT_INTERVAL_MS (500)
#define DHT_MEASUREMENT_COUNT 10
#define ONBOARD_LED 2
#define PROX_INPUT 35
#define DHT_INPUT 4
#define DHTTYPE DHT22

/***** LOCAL VALUES *****/
DHT dht(DHT_INPUT, DHTTYPE);
static WiFiClient espClient;
static PubSubClient client(device_config._mqtt_server, 1883, espClient);
static int loop_count = 0;

/***** LOCAL FUNCTIONS *****/
static void setup_wifi(void);
static void setup_ota(void);
static void setup_board(void);
static void update_prox(void);
static void update_dht(void);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  if (client.connect(device_config._clientID)) {
    Serial.println("Connected to MQTT Broker!");
  } else {
    Serial.println("Connection to MQTT Broker failed...");
  }
  setup_ota();
  setup_board();
}

void loop() {
  ArduinoOTA.handle();

  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  if (!client.connected()) {
    if (client.connect(device_config._clientID)) {
      Serial.println("Re-Connected to MQTT Broker!");
    } else {
      Serial.println("Still not connected to MQTT Broker...");
      goto cleanup;
    }
  }

  update_prox();
  if (DHT_MEASUREMENT_COUNT <= loop_count) {
    update_dht();
    loop_count = 0;
  }

cleanup:
  delay(MEASUREMENT_INTERVAL_MS);
  loop_count++;
}

static void update_prox(void) {
  if (digitalRead(PROX_INPUT)) {
    Serial.println("person");
    client.publish(device_config._mqtt_topic_prox, "person");
  } else {
    Serial.println("empty");
    client.publish(device_config._mqtt_topic_prox, "empty");
  }
}

static void update_dht(void) {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature(true);
  if (isnan(hum)) {
    Serial.println("Error reading hum");
    client.publish(device_config._mqtt_topic_hum, "1000");
  }
  if (isnan(temp)) {
    Serial.println("Error reading temp");
    client.publish(device_config._mqtt_topic_temp, "1000");
  }
  if (isnan(hum) || isnan(temp)) {
    Serial.println("Error reading DHT22");
  } else {
    Serial.println("Temp = " + String(temp));
    Serial.println("Hum = " + String(hum));
    client.publish(device_config._mqtt_topic_temp, String(temp).c_str());
    client.publish(device_config._mqtt_topic_hum, String(hum).c_str());
  }
}

static void setup_wifi(void) {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(device_config._ssid);

  WiFi.begin(device_config._ssid, device_config._password);
  bool ledState = true;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(ONBOARD_LED, ledState);  // Also onboard LED
    ledState = !ledState;  // Flip ledState
  }
  digitalWrite(ONBOARD_LED, HIGH);  // WiFi connected

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

static void setup_board(void) {
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(PROX_INPUT, INPUT);
  dht.begin();
}

void setup_ota() {
  ArduinoOTA.setHostname(device_config._hostName);
  ArduinoOTA.setPassword(device_config._otaPass);
  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else  // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS
        // using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();
}
