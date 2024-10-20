typedef struct {
  const char *_ssid;
  const char *_password;
  const char *_mqtt_server;
  const char *_mqtt_topic_prox;
  const char *_mqtt_topic_hum;
  const char *_mqtt_topic_temp;
  const char *_clientID;
  const char *_hostName;
  const char *_otaPass;
  const float _temperature_offset_F;
} config_t;

const config_t device_config = {._ssid = "SSID",
    ._password = "PASSWORD",
    ._mqtt_server = "192.168.4.4",
    ._mqtt_topic_prox = "myHome/room/prox",
    ._mqtt_topic_hum = "myHome/room/hum",
    ._mqtt_topic_temp = "myHome/room/temp",
    ._clientID = "Room Sensor",
    ._hostName = "Room Sensor",
    ._otaPass = "PASSWORD",
    ._temperature_offset_F = -1.1};