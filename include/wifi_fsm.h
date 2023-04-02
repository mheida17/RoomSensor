#pragma once

#include <WiFi.h>

#include "fsm.h"

typedef enum {
  WIFI_EVENT_START = FSM_GLOBAL_EVENT_COUNT,
  WIFI_EVENT_STOP,
  WIFI_EVENT_UNAVAILABLE,
} local_wifi_event_t;

/**
 * @brief Initialize the WiFi state machine
 *
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t wifi_fsm_init(uint8_t led_pin);

/**
 * @brief Send an event to the WiFi state machine
 *
 * @param event The specific event
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t wifi_fsm_send(fsm_event event);

/**
 * @brief Handle any pending events in the WiFi state machine
 *
 * @return fsm_err_t FSM_ERR_OK on success, relevant error otherwise
 */
fsm_err_t wifi_fsm_handle_event(void);

WiFiClient *wifi_fsm_get_client(void);