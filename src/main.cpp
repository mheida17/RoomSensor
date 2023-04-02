#include <Arduino.h>
#include <Config.h>

#include "dht_fsm.h"
#include "mqtt_fsm.h"
#include "ota_handler.h"
#include "prox_fsm.h"
#include "wifi_fsm.h"

/***** DEFINES *****/
#define MEASUREMENT_INTERVAL_MS (500)
#define ONBOARD_LED 2
#define SERIAL_SPEED 115200

void setup() {
  Serial.begin(SERIAL_SPEED);

  pinMode(ONBOARD_LED, OUTPUT);
  delay(3000);

  dht_fsm_init();
  prox_fsm_init();
  wifi_fsm_init(ONBOARD_LED);
  mqtt_fsm_init();
  setup_ota();
}

static void send_periodic_events(fsm_event event) {
  dht_fsm_send(event);
  prox_fsm_send(event);
  wifi_fsm_send(event);
  mqtt_fsm_send(event);
}

void handle_events(void) {
  fsm_err_t retVal = FSM_ERR_OK;
  while (FSM_ERR_NO_EVENTS != retVal && FSM_ERR_OK == retVal) {
    retVal = wifi_fsm_handle_event();
  }
  retVal = FSM_ERR_OK;
  while (FSM_ERR_NO_EVENTS != retVal && FSM_ERR_OK == retVal) {
    retVal = mqtt_fsm_handle_event();
  }
  retVal = FSM_ERR_OK;
  while (FSM_ERR_NO_EVENTS != retVal && FSM_ERR_OK == retVal) {
    retVal = prox_fsm_handle_event();
  }
  retVal = FSM_ERR_OK;
  while (FSM_ERR_NO_EVENTS != retVal && FSM_ERR_OK == retVal) {
    retVal = dht_fsm_handle_event();
  }
}

void loop() {
  static uint16_t elapsed_time_ms = 0;
  ota_handler();

  if (0 == elapsed_time_ms % MEASUREMENT_INTERVAL_MS) {
    send_periodic_events(FSM_PERIODIC_EVENT_500MS);
  }
  if (0 == elapsed_time_ms % 1000) {
    send_periodic_events(FSM_PERIODIC_EVENT_1S);
  }
  if (0 == elapsed_time_ms % 5000) {
    send_periodic_events(FSM_PERIODIC_EVENT_5S);
  }
  handle_events();
  delay(MEASUREMENT_INTERVAL_MS);
  elapsed_time_ms = (elapsed_time_ms + MEASUREMENT_INTERVAL_MS) % 5000;
}
