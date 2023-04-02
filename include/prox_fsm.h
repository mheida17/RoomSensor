#pragma once

#include "fsm.h"

typedef enum {
  PROX_EVENT_START = FSM_GLOBAL_EVENT_COUNT,
  PROX_EVENT_STOP,
  PROX_EVENT_UNAVAILABLE,
} prox_event_t;

/**
 * @brief Initialize the Proximity state machine
 *
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t prox_fsm_init(void);

/**
 * @brief Send an event to the Proximity state machine
 *
 * @param event The specific event
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t prox_fsm_send(fsm_event event);

/**
 * @brief Handle any pending events in the Proximity state machine
 *
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t prox_fsm_handle_event(void);

void prox_set_IRQ(bool enable);
bool get_prox(void);