#include "capture_controller.h"

void capture_controller_run(capture_controller_ctx_t* ctx,
                            const capture_controller_ops_t* ops)
{
  switch (*ctx->capture_sm_state)
  {
    case CAPTURE_SM_IDLE:
      if (*ctx->usb_capture_request_pending != 0U)
      {
        *ctx->usb_capture_request_pending = 0U;
        *ctx->capture_request_active = 1U;
        *ctx->capture_recovery_attempted = 0U;
        *ctx->capture_sm_state = CAPTURE_SM_ARM_CAPTURE;
      }
      break;

    case CAPTURE_SM_ARM_CAPTURE:
      if (ops->start_capture_once() != 0U)
      {
        *ctx->capture_sm_state = CAPTURE_SM_WAIT_FRAME;
      }
      else
      {
        *ctx->capture_sm_state = CAPTURE_SM_ERROR_RECOVERY;
      }
      break;

    case CAPTURE_SM_WAIT_FRAME:
      if (*ctx->dcmi_error_flag != 0U)
      {
        *ctx->capture_sm_state = CAPTURE_SM_ERROR_RECOVERY;
      }
      else if (*ctx->all_frames_ready != 0U)
      {
        *ctx->capture_sm_state = CAPTURE_SM_TX_FRAME;
      }
      else if ((ops->get_tick_ms() - *ctx->capture_wait_start_ms) > ctx->capture_wait_timeout_ms)
      {
        (*ctx->frame_timeout_count)++;
        if (*ctx->capture_recovery_attempted == 0U)
        {
          *ctx->capture_recovery_attempted = 1U;
          *ctx->capture_sm_state = CAPTURE_SM_ERROR_RECOVERY;
        }
        else
        {
          ops->dcmi_stop_abort_and_clear();
          *ctx->capture_request_active = 0U;
          *ctx->capture_sm_state = CAPTURE_SM_IDLE;
        }
      }
      break;

    case CAPTURE_SM_TX_FRAME:
      if (ops->process_and_transmit_data() != 0U)
      {
        (*ctx->tx_frame_ok_count)++;
      }
      else
      {
        (*ctx->tx_frame_abort_count)++;
      }

      *ctx->capture_request_active = 0U;
      *ctx->capture_recovery_attempted = 0U;
      *ctx->capture_sm_state = CAPTURE_SM_IDLE;
      break;

    case CAPTURE_SM_ERROR_RECOVERY:
      ops->handle_dcmi_error();
      if (*ctx->capture_request_active != 0U)
      {
        *ctx->capture_sm_state = CAPTURE_SM_ARM_CAPTURE;
      }
      else
      {
        *ctx->capture_sm_state = CAPTURE_SM_IDLE;
      }
      break;

    default:
      *ctx->capture_sm_state = CAPTURE_SM_IDLE;
      break;
  }
}

const char* capture_controller_state_to_string(capture_sm_state_t state)
{
  switch (state)
  {
    case CAPTURE_SM_IDLE:
      return "IDLE";
    case CAPTURE_SM_ARM_CAPTURE:
      return "ARM";
    case CAPTURE_SM_WAIT_FRAME:
      return "WAIT";
    case CAPTURE_SM_TX_FRAME:
      return "TX";
    case CAPTURE_SM_ERROR_RECOVERY:
      return "REC";
    default:
      return "UNK";
  }
}
