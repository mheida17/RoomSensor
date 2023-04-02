#include "prox_fsm.h"

#include <Arduino.h>

#include "HardwareSerial.h"

static fsm_handle_t state_machine;

enum { PROX_UNKNOWN, PROX_ACTIVE, PROX_INACTIVE };

#define PROX_INPUT 35
#define ENABLE true
#define DISABLE false

#define MOTION_INTERVAL_MS (120 * 1000)
#define MOTION_COUNT_MAX 3

typedef enum {
  PROX_DETECTED,
  PROX_NOT_DETECTED,
} proximityState_t;

typedef struct {
  unsigned long timestamp_ms;
  size_t motion_count;
  proximityState_t prox_state;
  proximityState_t previous_prox_state;
} motionEvent_t;

volatile motionEvent_t latestMotion = {.timestamp_ms = 0,
    .motion_count = 0,
    .prox_state = PROX_NOT_DETECTED,
    .previous_prox_state = PROX_NOT_DETECTED};

static bool person_detected = false;

/**
 * @brief ISR when motion is detected
 *
 * The trigger lasts for 1 second and can't be triggered again for 4-6 seconds
 */
void IRAM_ATTR motion_detected() {
  latestMotion.motion_count +=
      (latestMotion.motion_count < MOTION_COUNT_MAX) ? 1 : 0;
  latestMotion.timestamp_ms = millis();
}

void prox_set_IRQ(bool enable) {
  if (enable) {
    attachInterrupt(PROX_INPUT, motion_detected, RISING);
  } else {
    detachInterrupt(PROX_INPUT);
  }
}

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
    {.destination_state_ID = PROX_ACTIVE, .event = PROX_EVENT_START},
    {.destination_state_ID = PROX_INACTIVE, .event = PROX_EVENT_STOP},
};

static fsm_transition_t active_transitions[] = {
    {.destination_state_ID = PROX_INACTIVE, .event = PROX_EVENT_STOP},
    {.destination_state_ID = PROX_ACTIVE,
        .event = FSM_PERIODIC_EVENT_5S,
        .transition_fn = periodic_active_event_fn}};

static fsm_transition_t inactive_transitions[] = {
    {.destination_state_ID = PROX_ACTIVE, .event = PROX_EVENT_START}};

/******** STATES ********/
static fsm_state_t unknown_state = {.ID = PROX_UNKNOWN,
    .entry_fn = unknown_entry_fn,
    .exit_fn = unknown_exit_fn,
    .transition_array = unknown_transitions,
    .num_transitions =
        sizeof(unknown_transitions) / sizeof(unknown_transitions[0])};

static fsm_state_t active_state = {.ID = PROX_ACTIVE,
    .entry_fn = active_entry_fn,
    .exit_fn = active_exit_fn,
    .transition_array = active_transitions,
    .num_transitions =
        sizeof(active_transitions) / sizeof(active_transitions[0])};

static fsm_state_t inactive_state = {.ID = PROX_INACTIVE,
    .entry_fn = inactive_entry_fn,
    .exit_fn = inactive_exit_fn,
    .transition_array = inactive_transitions,
    .num_transitions =
        sizeof(inactive_transitions) / sizeof(inactive_transitions[0])};

/******** STATE MACHINE ********/
static fsm_state_t states[] = {
    [PROX_UNKNOWN] = unknown_state,
    [PROX_ACTIVE] = active_state,
    [PROX_INACTIVE] = inactive_state,
};

/******** PUBLIC FUNCTIONS ********/
fsm_err_t prox_fsm_init(void) {
  return fsm_init(&state_machine, states, sizeof(states) / sizeof(states[0]));
}

fsm_err_t prox_fsm_send(fsm_event event) {
  return fsm_send(&state_machine, event);
}

fsm_err_t prox_fsm_handle_event(void) {
  return fsm_handle_event(&state_machine);
}

bool get_prox(void) { return person_detected; }

/******** PRIVATE FUNCTIONS ********/
static fsm_err_t unknown_entry_fn() {
  pinMode(PROX_INPUT, INPUT);
  prox_fsm_send(PROX_EVENT_START);
  return FSM_ERR_OK;
}

static fsm_err_t active_entry_fn() {
  prox_set_IRQ(ENABLE);
  return FSM_ERR_OK;
}

static fsm_err_t inactive_entry_fn() {
  prox_set_IRQ(DISABLE);
  return FSM_ERR_OK;
}

static fsm_err_t periodic_active_event_fn() {
  if (latestMotion.motion_count > 0) {
    if (millis() - latestMotion.timestamp_ms > MOTION_INTERVAL_MS) {
      latestMotion.motion_count--;
      latestMotion.timestamp_ms = millis();
    }
    if (PROX_DETECTED != latestMotion.previous_prox_state) {
      person_detected = true;
    }
    latestMotion.previous_prox_state = PROX_DETECTED;
  } else {
    if (PROX_NOT_DETECTED != latestMotion.previous_prox_state) {
      person_detected = false;
    }
    latestMotion.previous_prox_state = PROX_NOT_DETECTED;
  }
  return FSM_ERR_OK;
}

/******** NO OP FUNCTIONS ********/
static fsm_err_t unknown_exit_fn() { return FSM_ERR_OK; }
static fsm_err_t active_exit_fn() { return FSM_ERR_OK; }
static fsm_err_t inactive_exit_fn() { return FSM_ERR_OK; }
