#include "mqtt_fsm.h"

#include <Arduino.h>
#include <CircularBuffer.h>
#include <PubSubClient.h>

#include "Config.h"
#include "dht_fsm.h"
#include "prox_fsm.h"
#include "wifi_fsm.h"

static fsm_handle_t state_machine;

enum { MQTT_UNKNOWN, MQTT_ACTIVE, MQTT_INACTIVE };

static PubSubClient client(
    device_config._mqtt_server, 1883, *wifi_fsm_get_client());

static bool messages_available = true;

// CircularBuffer<String, 32> topics;
// CircularBuffer<String, 32> messages;

/******** PRIVATE FUNCTIONS ********/
static fsm_err_t unknown_entry_fn();
static fsm_err_t unknown_exit_fn();
static fsm_err_t active_entry_fn();
static fsm_err_t active_exit_fn();
static fsm_err_t inactive_entry_fn();
static fsm_err_t inactive_exit_fn();

static fsm_err_t periodic_inactive_event_fn();
static fsm_err_t periodic_active_event_fn();

/******** TRANSITIONS ********/
static fsm_transition_t unknown_transitions[] = {
    {.destination_state_ID = MQTT_ACTIVE, .event = MQTT_EVENT_START},
    {.destination_state_ID = MQTT_INACTIVE, .event = MQTT_EVENT_STOP},
};

static fsm_transition_t active_transitions[] = {
    {.destination_state_ID = MQTT_INACTIVE, .event = MQTT_EVENT_STOP},
    {.destination_state_ID = MQTT_ACTIVE,
        .event = FSM_PERIODIC_EVENT_5S,
        .transition_fn = periodic_active_event_fn},
};

static fsm_transition_t inactive_transitions[] = {
    {.destination_state_ID = MQTT_ACTIVE, .event = MQTT_EVENT_START},
    {.destination_state_ID = MQTT_INACTIVE,
        .event = FSM_PERIODIC_EVENT_1S,
        .transition_fn = periodic_inactive_event_fn}};

/******** STATES ********/
static fsm_state_t unknown_state = {.ID = MQTT_UNKNOWN,
    .entry_fn = unknown_entry_fn,
    .exit_fn = unknown_exit_fn,
    .transition_array = unknown_transitions,
    .num_transitions =
        sizeof(unknown_transitions) / sizeof(unknown_transitions[0])};

static fsm_state_t active_state = {.ID = MQTT_ACTIVE,
    .entry_fn = active_entry_fn,
    .exit_fn = active_exit_fn,
    .transition_array = active_transitions,
    .num_transitions =
        sizeof(active_transitions) / sizeof(active_transitions[0])};

static fsm_state_t inactive_state = {.ID = MQTT_INACTIVE,
    .entry_fn = inactive_entry_fn,
    .exit_fn = inactive_exit_fn,
    .transition_array = inactive_transitions,
    .num_transitions =
        sizeof(inactive_transitions) / sizeof(inactive_transitions[0])};

/******** STATE MACHINE ********/
static fsm_state_t states[] = {
    [MQTT_UNKNOWN] = unknown_state,
    [MQTT_ACTIVE] = active_state,
    [MQTT_INACTIVE] = inactive_state,
};

/******** PUBLIC FUNCTIONS ********/
fsm_err_t mqtt_fsm_init(void) {
  return fsm_init(&state_machine, states, sizeof(states) / sizeof(states[0]));
}

fsm_err_t mqtt_fsm_send(fsm_event event) {
  return fsm_send(&state_machine, event);
}

fsm_err_t mqtt_fsm_handle_event(void) {
  return fsm_handle_event(&state_machine);
}

// fsm_err_t mqtt_fsm_queue_msg(const char topic[20], const char val[20]) {
//   messages.push(val);
//   topics.push(topic);
//   messages_available = true;
//   return FSM_ERR_OK;
// }

/******** PRIVATE FUNCTIONS ********/
static fsm_err_t unknown_entry_fn() {
  if (client.connect(device_config._clientID)) {
    Serial.println("Connected to MQTT Broker!");
    mqtt_fsm_send(MQTT_EVENT_START);
  } else {
    Serial.println("Connection to MQTT Broker failed...");
    mqtt_fsm_send(MQTT_EVENT_STOP);
  }
  return FSM_ERR_OK;
}

static fsm_err_t periodic_inactive_event_fn() {
  if (client.connect(device_config._clientID)) {
    Serial.println("Connected to MQTT Broker!");
    mqtt_fsm_send(MQTT_EVENT_START);
  } else {
    Serial.println("Connection to MQTT Broker failed...");
  }
  return FSM_ERR_OK;
}

static fsm_err_t periodic_active_event_fn() {
  if (!client.connected()) {
    mqtt_fsm_send(MQTT_EVENT_STOP);
    return FSM_ERR_OK;
  }

  bool reactivate_prox = false;
  if (messages_available) {
    reactivate_prox = true;
    prox_fsm_send(PROX_EVENT_STOP);
    fsm_err_t retVal = FSM_ERR_OK;
    while (FSM_ERR_NO_EVENTS != retVal && FSM_ERR_OK == retVal) {
      retVal = prox_fsm_handle_event();
    }
  }

  // TODO: create a buffer for messages, trigger off watermark
  client.publish(device_config._mqtt_topic_hum, String(get_hum()).c_str());
  client.publish(device_config._mqtt_topic_temp, String(get_temp()).c_str());
  client.publish(
      device_config._mqtt_topic_prox, (get_prox()) ? "person" : "empty");

  if (reactivate_prox) {
    prox_fsm_send(PROX_EVENT_START);
    fsm_err_t retVal = FSM_ERR_OK;
    while (FSM_ERR_NO_EVENTS != retVal && FSM_ERR_OK == retVal) {
      retVal = prox_fsm_handle_event();
    }
  }

  return FSM_ERR_OK;
}

/******** NO OP FUNCTIONS ********/
static fsm_err_t unknown_exit_fn() { return FSM_ERR_OK; }
static fsm_err_t active_entry_fn() { return FSM_ERR_OK; }
static fsm_err_t active_exit_fn() { return FSM_ERR_OK; }
static fsm_err_t inactive_entry_fn() { return FSM_ERR_OK; }
static fsm_err_t inactive_exit_fn() { return FSM_ERR_OK; }