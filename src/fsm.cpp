#include "fsm.h"

#include <stdio.h>
#include <string.h>

#include "HardwareSerial.h"

/************* Public Functions *************/
fsm_err_t fsm_init(
    fsm_handle *state_machine, fsm_state_t *states, uint8_t num_states) {
  if (!state_machine || !states || 0 == num_states) {
    Serial.println("Bad Input");
    return FSM_ERR_EINVAL;
  }
  state_machine->state_array = states;
  state_machine->num_states = num_states;
  memset(
      state_machine->pending_events, 0, sizeof(state_machine->pending_events));
  state_machine->num_pending_events = 0;
  state_machine->current_state_ID = 0;
  state_machine->state_array[state_machine->current_state_ID].entry_fn();
  return FSM_ERR_OK;
}

fsm_err_t fsm_send(fsm_handle *state_machine, fsm_event event) {
  if (!state_machine) {
    return FSM_ERR_EINVAL;
  }
  if (MAX_PENDING_EVENTS <= state_machine->num_pending_events) {
    return FSM_ERR_FULL;
  }
  state_machine->pending_events[state_machine->num_pending_events++] = event;
  return FSM_ERR_OK;
}

fsm_err_t fsm_handle_event(fsm_handle *state_machine) {
  if (!state_machine) {
    return FSM_ERR_EINVAL;
  }
  if (0 == state_machine->num_pending_events) {
    return FSM_ERR_NO_EVENTS;
  }

  fsm_state_t current_state =
      state_machine->state_array[state_machine->current_state_ID];
  fsm_event current_event = state_machine->pending_events[0];

  for (size_t i = 0; i < state_machine->num_pending_events - 1; i++) {
    state_machine->pending_events[i] = state_machine->pending_events[i + 1];
  }

  state_machine->num_pending_events--;
  state_machine->pending_events[state_machine->num_pending_events] = 0;

  for (size_t i = 0; i < current_state.num_transitions; i++) {
    if (current_event == current_state.transition_array[i].event) {
      fsm_state_t desired_state =
          state_machine->state_array[current_state.transition_array[i]
                                         .destination_state_ID];
      if (state_machine->current_state_ID ==
          current_state.transition_array[i].destination_state_ID) {
        if (current_state.transition_array[i].transition_fn &&
            0 != current_state.transition_array[i].transition_fn()) {
          return FSM_ERR_TRANS;
        }
      } else {
        if (current_state.exit_fn && 0 != current_state.exit_fn()) {
          return FSM_ERR_TRANS;
        }
        if (desired_state.entry_fn && 0 != desired_state.entry_fn()) {
          return FSM_ERR_TRANS;
        }
      }
      state_machine->current_state_ID = desired_state.ID;
      break;
    }
  }

  return FSM_ERR_OK;
}
