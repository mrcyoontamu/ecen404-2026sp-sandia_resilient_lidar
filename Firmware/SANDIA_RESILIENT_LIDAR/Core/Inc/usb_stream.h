#ifndef USB_STREAM_H
#define USB_STREAM_H

#include <stdint.h>

uint8_t usb_stream_transmit_blocking(uint8_t* buf, uint16_t len);
uint8_t usb_stream_transmit_frame(uint16_t* frame_pointer,
                                  uint32_t single_frame_size_pixels);
uint8_t usb_stream_transmit_quad_frame(uint16_t* quad_frame_buffer,
                                       uint32_t single_frame_size_pixels,
                                       uint32_t num_frames);

#endif
