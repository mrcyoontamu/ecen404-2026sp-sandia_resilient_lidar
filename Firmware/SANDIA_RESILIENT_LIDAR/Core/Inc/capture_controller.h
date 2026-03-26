#ifndef CAPTURE_CONTROLLER_H
#define CAPTURE_CONTROLLER_H

#include <stdint.h>

typedef enum
{
  CAPTURE_SM_IDLE = 0,
  CAPTURE_SM_ARM_CAPTURE,
  CAPTURE_SM_WAIT_FRAME,
  CAPTURE_SM_TX_FRAME,
  CAPTURE_SM_ERROR_RECOVERY
} capture_sm_state_t;

typedef struct
{
  volatile capture_sm_state_t* capture_sm_state;
  volatile uint8_t* usb_capture_request_pending;
  volatile uint8_t* capture_request_active;
  volatile uint8_t* capture_recovery_attempted;
  volatile uint8_t* dcmi_error_flag;
  volatile uint8_t* all_frames_ready;
  volatile uint32_t* capture_wait_start_ms;
  volatile uint32_t* frame_timeout_count;
  volatile uint32_t* tx_frame_ok_count;
  volatile uint32_t* tx_frame_abort_count;
  uint32_t capture_wait_timeout_ms;
} capture_controller_ctx_t;

typedef struct
{
  uint8_t (*start_capture_once)(void);
  uint8_t (*process_and_transmit_data)(void);
  void (*handle_dcmi_error)(void);
  void (*dcmi_stop_abort_and_clear)(void);
  uint32_t (*get_tick_ms)(void);
} capture_controller_ops_t;

void capture_controller_run(capture_controller_ctx_t* ctx,
                            const capture_controller_ops_t* ops);
const char* capture_controller_state_to_string(capture_sm_state_t state);

#endif
