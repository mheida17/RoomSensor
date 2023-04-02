#include "dht_fsm.h"

#include <DHT.h>
#include <stdio.h>

#define DHTTYPE DHT22
#define DHT_INPUT 4
DHT dht(DHT_INPUT, DHTTYPE);

static fsm_handle_t state_machine;

enum { DHT_UNKNOWN, DHT_ACTIVE, DHT_INACTIVE };
static int current_humidity = 1000;
static int current_temp = 1000;

/******** PRIVATE FUNCTIONS ********/
static fsm_err_t unknown_entry_fn();
static fsm_err_t unknown_exit_fn();
static fsm_err_t active_entry_fn();
static fsm_err_t active_exit_fn();
static fsm_err_t inactive_entry_fn();
static fsm_err_t inactive_exit_fn();
static fsm_err_t periodic_active_event_fn();

/******** TRANSITIONS ********/
static fsm_transition_t unknown_transitions[] = {
    {.destination_state_ID = DHT_ACTIVE, .event = DHT_EVENT_START},
    {.destination_state_ID = DHT_INACTIVE, .event = DHT_EVENT_STOP},
};

static fsm_transition_t active_transitions[] = {
    {.destination_state_ID = DHT_INACTIVE, .event = DHT_EVENT_STOP},
    {.destination_state_ID = DHT_ACTIVE,
        .event = FSM_PERIODIC_EVENT_5S,
        .transition_fn = periodic_active_event_fn}};

static fsm_transition_t inactive_transitions[] = {
    {.destination_state_ID = DHT_ACTIVE, .event = DHT_EVENT_START}};

/******** STATES ********/
static fsm_state_t unknown_state = {.ID = DHT_UNKNOWN,
    .entry_fn = unknown_entry_fn,
    .exit_fn = unknown_exit_fn,
    .transition_array = unknown_transitions,
    .num_transitions =
        sizeof(unknown_transitions) / sizeof(unknown_transitions[0])};

static fsm_state_t active_state = {.ID = DHT_ACTIVE,
    .entry_fn = active_entry_fn,
    .exit_fn = active_exit_fn,
    .transition_array = active_transitions,
    .num_transitions =
        sizeof(active_transitions) / sizeof(active_transitions[0])};

static fsm_state_t inactive_state = {.ID = DHT_INACTIVE,
    .entry_fn = inactive_entry_fn,
    .exit_fn = inactive_exit_fn,
    .transition_array = inactive_transitions,
    .num_transitions =
        sizeof(inactive_transitions) / sizeof(inactive_transitions[0])};

/******** STATE MACHINE ********/
static fsm_state_t states[] = {
    [DHT_UNKNOWN] = unknown_state,
    [DHT_ACTIVE] = active_state,
    [DHT_INACTIVE] = inactive_state,
};

/******** PUBLIC FUNCTIONS ********/
fsm_err_t dht_fsm_init(void) {
  return fsm_init(&state_machine, states, sizeof(states) / sizeof(states[0]));
}

fsm_err_t dht_fsm_send(fsm_event event) {
  return fsm_send(&state_machine, event);
}

fsm_err_t dht_fsm_handle_event(void) {
  return fsm_handle_event(&state_machine);
}

int get_temp(void) { return current_temp; }

int get_hum(void) { return current_humidity; }
/******** PRIVATE FUNCTIONS ********/
static fsm_err_t unknown_entry_fn() {
  dht.begin();
  if (3 == DEVICE_LOC) {
    // Living room hardware requires power workaround
    pinMode(27, OUTPUT);
    digitalWrite(27, HIGH);
  }
  dht_fsm_send(DHT_EVENT_START);
  return FSM_ERR_OK;
}

static fsm_err_t periodic_active_event_fn() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature(true);
  if (isnan(hum)) {
    Serial.println("Error reading hum");
    current_humidity = 1000;
  }
  if (isnan(temp)) {
    Serial.println("Error reading temp");
    current_temp = 1000;
  }
  if (isnan(hum) || isnan(temp)) {
    Serial.println("Error reading DHT22");
  } else {
    current_temp = temp - TEMPERATURE_OFFSET;
    current_humidity = hum;
  }
  return FSM_ERR_OK;
}

/******** NO OP FUNCTIONS ********/
static fsm_err_t unknown_exit_fn() { return FSM_ERR_OK; }
static fsm_err_t active_entry_fn() { return FSM_ERR_OK; }
static fsm_err_t active_exit_fn() { return FSM_ERR_OK; }
static fsm_err_t inactive_entry_fn() { return FSM_ERR_OK; }
static fsm_err_t inactive_exit_fn() { return FSM_ERR_OK; }
