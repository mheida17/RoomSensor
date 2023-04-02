#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
  FSM_ERR_EVENT_HANDLED = 2,
  FSM_ERR_NO_EVENTS = 1,
  FSM_ERR_OK = 0,
  FSM_ERR_EINVAL = -1,
  FSM_ERR_FULL = -2,
  FSM_ERR_TRANS = -3,
} fsm_err_t;

enum {
  FSM_PERIODIC_EVENT_500MS,
  FSM_PERIODIC_EVENT_1S,
  FSM_PERIODIC_EVENT_5S,
  FSM_GLOBAL_EVENT_COUNT
};

#define MAX_PENDING_EVENTS 32

typedef uint16_t fsm_event;
typedef uint16_t fsm_state_ID;

typedef fsm_err_t (*fsm_function)(void);

typedef struct fsm_transition {
  fsm_state_ID destination_state_ID;
  fsm_event event;
  fsm_function transition_fn;
} fsm_transition_t;

typedef struct fsm_state {
  fsm_state_ID ID;
  fsm_function entry_fn;
  fsm_function exit_fn;
  fsm_transition_t *transition_array;
  size_t num_transitions;
} fsm_state_t;

typedef struct fsm_handle {
  fsm_state_t *state_array;
  uint8_t num_states;
  fsm_state_ID current_state_ID;
  fsm_event pending_events[MAX_PENDING_EVENTS];
  uint8_t num_pending_events;
} fsm_handle_t;

/**
 * @brief Initialize a Finite State Machine
 *
 * @param state_machine the handle for the state machine
 * @param states an array of state definitions
 * @param num_states the number of states
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t fsm_init(
    fsm_handle_t *state_machine, fsm_state_t *states, uint8_t num_states);

/**
 * @brief Send an event to a state machine's queue
 *
 * @param state_machine the handle for the state machine
 * @param event a common or specific event for the state machine
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t fsm_send(fsm_handle_t *state_machine, fsm_event event);

/**
 * @brief Handle any pending events in the state machine's queue
 *
 * @param state_machine the handle for the state machine
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t fsm_handle_event(fsm_handle_t *state_machine);
