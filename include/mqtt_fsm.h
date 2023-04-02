#pragma once

#include "fsm.h"

typedef enum {
  MQTT_EVENT_START = FSM_GLOBAL_EVENT_COUNT,
  MQTT_EVENT_STOP,
  MQTT_EVENT_UNAVAILABLE,
} mqtt_event_t;

/**
 * @brief Initialize the MQTT state machine
 *
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t mqtt_fsm_init(void);

/**
 * @brief Send an event to the MQTT state machine
 *
 * @param event The specific event
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t mqtt_fsm_send(fsm_event event);

/**
 * @brief Handle any pending events in the MQTT state machine
 *
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t mqtt_fsm_handle_event(void);

/* fsm_err_t mqtt_fsm_queue_msg(const char topic[20], const char val[20]); */