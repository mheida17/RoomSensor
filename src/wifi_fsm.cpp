#include "wifi_fsm.h"

#include <Arduino.h>

#include "Config.h"

static uint8_t led_pin_s = 0;

static fsm_handle_t state_machine;

WiFiClient espClient;

enum { WIFI_UNKNOWN, WIFI_ACTIVE, WIFI_INACTIVE };

/******** PRIVATE FUNCTIONS ********/
static fsm_err_t unknown_entry_fn();
static fsm_err_t unknown_exit_fn();
static fsm_err_t active_entry_fn();
static fsm_err_t active_exit_fn();
static fsm_err_t inactive_entry_fn();
static fsm_err_t inactive_exit_fn();

static fsm_err_t periodic_active_event_fn();
static fsm_err_t periodic_inactive_event_fn();

/******** TRANSITIONS ********/
static fsm_transition_t unknown_transitions[] = {
    {.destination_state_ID = WIFI_ACTIVE, .event = WIFI_EVENT_START},
    {.destination_state_ID = WIFI_INACTIVE, .event = WIFI_EVENT_STOP},
};

static fsm_transition_t active_transitions[] = {
    {.destination_state_ID = WIFI_INACTIVE, .event = WIFI_EVENT_STOP},
    {.destination_state_ID = WIFI_ACTIVE, .event = FSM_PERIODIC_EVENT_1S},
};

static fsm_transition_t inactive_transitions[] = {
    {.destination_state_ID = WIFI_ACTIVE, .event = WIFI_EVENT_START},
    {.destination_state_ID = WIFI_INACTIVE,
        .event = FSM_PERIODIC_EVENT_5S,
        .transition_fn = periodic_inactive_event_fn}};

/******** STATES ********/
static fsm_state_t unknown_state = {.ID = WIFI_UNKNOWN,
    .entry_fn = unknown_entry_fn,
    .exit_fn = unknown_exit_fn,
    .transition_array = unknown_transitions,
    .num_transitions =
        sizeof(unknown_transitions) / sizeof(unknown_transitions[0])};

static fsm_state_t active_state = {.ID = WIFI_ACTIVE,
    .entry_fn = active_entry_fn,
    .exit_fn = active_exit_fn,
    .transition_array = active_transitions,
    .num_transitions =
        sizeof(active_transitions) / sizeof(active_transitions[0])};

static fsm_state_t inactive_state = {.ID = WIFI_INACTIVE,
    .entry_fn = inactive_entry_fn,
    .exit_fn = inactive_exit_fn,
    .transition_array = inactive_transitions,
    .num_transitions =
        sizeof(inactive_transitions) / sizeof(inactive_transitions[0])};

/******** STATE MACHINE ********/
static fsm_state_t states[] = {
    [WIFI_UNKNOWN] = unknown_state,
    [WIFI_ACTIVE] = active_state,
    [WIFI_INACTIVE] = inactive_state,
};

/******** PUBLIC FUNCTIONS ********/
fsm_err_t wifi_fsm_init(uint8_t led_pin) {
  led_pin_s = led_pin;
  return fsm_init(&state_machine, states, sizeof(states) / sizeof(states[0]));
}

fsm_err_t wifi_fsm_send(fsm_event event) {
  return fsm_send(&state_machine, event);
}

fsm_err_t wifi_fsm_handle_event(void) {
  return fsm_handle_event(&state_machine);
}

WiFiClient *wifi_fsm_get_client(void) { return &espClient; }

/******** PRIVATE FUNCTIONS ********/
static void wifi_connect() {
  Serial.print("Connecting to ");
  Serial.println(device_config._ssid);

  WiFi.begin(device_config._ssid, device_config._password);
  bool ledState = true;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(led_pin_s, ledState);
    ledState = !ledState;
  }
  digitalWrite(led_pin_s, HIGH);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  wifi_fsm_send(WIFI_EVENT_START);
}

static fsm_err_t unknown_entry_fn() {
  wifi_connect();
  return FSM_ERR_OK;
}

static fsm_err_t periodic_inactive_event_fn() {
  wifi_connect();
  return FSM_ERR_OK;
}

static fsm_err_t periodic_active_event_fn() {
  if (WiFi.status() != WL_CONNECTED) {
    wifi_fsm_send(WIFI_EVENT_STOP);
  }
  return FSM_ERR_OK;
}

static fsm_err_t inactive_entry_fn() {
  wifi_connect();
  return FSM_ERR_OK;
}

/******** NO OP FUNCTIONS ********/
static fsm_err_t unknown_exit_fn() { return FSM_ERR_OK; }
static fsm_err_t active_entry_fn() { return FSM_ERR_OK; }
static fsm_err_t active_exit_fn() { return FSM_ERR_OK; }
static fsm_err_t inactive_exit_fn() { return FSM_ERR_OK; }
