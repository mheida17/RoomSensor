#pragma once

#include "fsm.h"

typedef enum {
  DHT_EVENT_START = FSM_GLOBAL_EVENT_COUNT,
  DHT_EVENT_STOP,
  DHT_EVENT_UNAVAILABLE,
} dht_event_t;

/**
 * @brief Initialize the DHT state machine
 *
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t dht_fsm_init(void);

/**
 * @brief Send an event to the DHT state machine
 *
 * @param event The specific event
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t dht_fsm_send(fsm_event event);

/**
 * @brief Handle any pending events in the DHT state machine
 *
 * @return fsm_err_t
 */
fsm_err_t dht_fsm_handle_event(void);

int get_temp(void);
int get_hum(void);