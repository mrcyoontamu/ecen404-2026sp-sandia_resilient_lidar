#include "usb_stream.h"

#include "usbd_cdc_if.h"

extern USBD_HandleTypeDef hUsbDeviceHS;

#define USB_STREAM_FRAME_HEADER_SIZE 4U
#define USB_STREAM_MAX_CHUNK_SIZE_BYTES 32768U

static const uint8_t k_usb_stream_frame_header[USB_STREAM_FRAME_HEADER_SIZE] = {0xAA, 0x55, 0xAA, 0x55};

uint8_t usb_stream_transmit_blocking(uint8_t* buf, uint16_t len)
{
  const uint32_t usb_tx_timeout_ms = 1000;
  uint32_t start = HAL_GetTick();

  if (hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED)
  {
    return 0;
  }

  USBD_CDC_HandleTypeDef* hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;

  while (hcdc->TxState != 0)
  {
    if ((HAL_GetTick() - start) > usb_tx_timeout_ms)
    {
      return 0;
    }
  }

  if (CDC_Transmit_HS(buf, len) != USBD_OK)
  {
    return 0;
  }

  start = HAL_GetTick();

  while (hcdc->TxState != 0)
  {
    if ((HAL_GetTick() - start) > usb_tx_timeout_ms)
    {
      return 0;
    }
  }

  return 1;
}

uint8_t usb_stream_transmit_frame(uint16_t* frame_pointer,
                                  uint32_t single_frame_size_pixels)
{
  uint8_t* p_buffer;
  uint32_t bytes_remaining;
  uint16_t chunk_size;

  if (hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED)
  {
    return 0;
  }

  if (usb_stream_transmit_blocking((uint8_t*)k_usb_stream_frame_header, USB_STREAM_FRAME_HEADER_SIZE) == 0)
  {
    return 0;
  }

  p_buffer = (uint8_t*)frame_pointer;
  bytes_remaining = single_frame_size_pixels * 2U;

  while (bytes_remaining > 0U)
  {
    if (bytes_remaining > USB_STREAM_MAX_CHUNK_SIZE_BYTES)
    {
      chunk_size = USB_STREAM_MAX_CHUNK_SIZE_BYTES;
    }
    else
    {
      chunk_size = (uint16_t)bytes_remaining;
    }

    if (usb_stream_transmit_blocking(p_buffer, chunk_size) == 0)
    {
      return 0;
    }

    p_buffer += chunk_size;
    bytes_remaining -= chunk_size;
  }

  return 1;
}

uint8_t usb_stream_transmit_quad_frame(uint16_t* quad_frame_buffer,
                                       uint32_t single_frame_size_pixels,
                                       uint32_t num_frames)
{
  uint8_t* p_buffer;
  uint32_t bytes_remaining;
  uint16_t chunk_size;

  if (hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED)
  {
    return 0;
  }

  if (usb_stream_transmit_blocking((uint8_t*)k_usb_stream_frame_header, USB_STREAM_FRAME_HEADER_SIZE) == 0)
  {
    return 0;
  }

  p_buffer = (uint8_t*)quad_frame_buffer;
  bytes_remaining = single_frame_size_pixels * num_frames * 2U;

  while (bytes_remaining > 0U)
  {
    if (bytes_remaining > USB_STREAM_MAX_CHUNK_SIZE_BYTES)
    {
      chunk_size = USB_STREAM_MAX_CHUNK_SIZE_BYTES;
    }
    else
    {
      chunk_size = (uint16_t)bytes_remaining;
    }

    if (usb_stream_transmit_blocking(p_buffer, chunk_size) == 0)
    {
      return 0;
    }

    p_buffer += chunk_size;
    bytes_remaining -= chunk_size;
  }

  return 1;
}
