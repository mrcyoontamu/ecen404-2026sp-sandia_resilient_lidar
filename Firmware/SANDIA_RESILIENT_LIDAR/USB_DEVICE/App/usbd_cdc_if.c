/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v1.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */

#include <string.h>

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

typedef enum
{
  USB_CMD_PARSE_WAIT_MAGIC_0 = 0,
  USB_CMD_PARSE_WAIT_MAGIC_1,
  USB_CMD_PARSE_VERSION,
  USB_CMD_PARSE_COMMAND,
  USB_CMD_PARSE_SEQUENCE,
  USB_CMD_PARSE_LENGTH,
  USB_CMD_PARSE_PAYLOAD,
  USB_CMD_PARSE_CRC_LO,
  USB_CMD_PARSE_CRC_HI
} usb_cmd_parse_state_t;

#define USB_CMD_QUEUE_LEN 8U

static volatile uint8_t s_usb_cmd_queue_head = 0U;
static volatile uint8_t s_usb_cmd_queue_tail = 0U;
static usb_cmd_packet_t s_usb_cmd_queue[USB_CMD_QUEUE_LEN];

static usb_cmd_parse_state_t s_usb_cmd_parse_state = USB_CMD_PARSE_WAIT_MAGIC_0;
static uint8_t s_usb_cmd_parse_version = 0U;
static uint8_t s_usb_cmd_parse_command = 0U;
static uint8_t s_usb_cmd_parse_sequence = 0U;
static uint8_t s_usb_cmd_parse_length = 0U;
static uint8_t s_usb_cmd_parse_payload[USB_CMD_MAX_PAYLOAD];
static uint8_t s_usb_cmd_parse_payload_index = 0U;
static uint16_t s_usb_cmd_parse_crc_rx = 0U;

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */

/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferHS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferHS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */
extern volatile uint8_t g_usb_capture_request_pending;
extern volatile uint8_t g_usb_status_request_pending;

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceHS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_HS(void);
static int8_t CDC_DeInit_HS(void);
static int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_HS(uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_HS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

static void usb_cmd_parse_reset(void);
static void usb_cmd_queue_reset(void);
static uint16_t usb_cmd_crc16_ccitt(const uint8_t* data, uint32_t length);
static void usb_cmd_queue_push(uint8_t command_id,
                               uint8_t sequence,
                               const uint8_t* payload,
                               uint8_t payload_len);
static void usb_cmd_parse_feed_byte(uint8_t byte);

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_HS =
{
  CDC_Init_HS,
  CDC_DeInit_HS,
  CDC_Control_HS,
  CDC_Receive_HS,
  CDC_TransmitCplt_HS
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the CDC media low layer over the USB HS IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_HS(void)
{
  /* USER CODE BEGIN 8 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceHS, UserTxBufferHS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceHS, UserRxBufferHS);
  usb_cmd_parse_reset();
  usb_cmd_queue_reset();
  return (USBD_OK);
  /* USER CODE END 8 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @param  None
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_HS(void)
{
  /* USER CODE BEGIN 9 */
  return (USBD_OK);
  /* USER CODE END 9 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 10 */
  switch(cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

  case CDC_SET_COMM_FEATURE:

    break;

  case CDC_GET_COMM_FEATURE:

    break;

  case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
  case CDC_SET_LINE_CODING:

    break;

  case CDC_GET_LINE_CODING:

    break;

  case CDC_SET_CONTROL_LINE_STATE:

    break;

  case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 10 */
}

/**
  * @brief Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAILL
  */
static int8_t CDC_Receive_HS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 11 */
  if ((Buf != NULL) && (Len != NULL) && (*Len > 0U))
  {
    for (uint32_t i = 0; i < *Len; i++)
    {
      // Keep legacy one-byte command support for compatibility.
      if ((Buf[i] == (uint8_t)'G') || (Buf[i] == (uint8_t)'g'))
      {
        g_usb_capture_request_pending = 1U;
      }
      else if ((Buf[i] == (uint8_t)'S') || (Buf[i] == (uint8_t)'s'))
      {
        g_usb_status_request_pending = 1U;
      }

      usb_cmd_parse_feed_byte(Buf[i]);
    }
  }

  USBD_CDC_SetRxBuffer(&hUsbDeviceHS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceHS);
  return (USBD_OK);
  /* USER CODE END 11 */
}

/**
  * @brief  Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_HS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 12 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceHS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceHS);
  /* USER CODE END 12 */
  return result;
}

/**
  * @brief  CDC_TransmitCplt_HS
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_TransmitCplt_HS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 14 */
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
  /* USER CODE END 14 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

static void usb_cmd_parse_reset(void)
{
  s_usb_cmd_parse_state = USB_CMD_PARSE_WAIT_MAGIC_0;
  s_usb_cmd_parse_version = 0U;
  s_usb_cmd_parse_command = 0U;
  s_usb_cmd_parse_sequence = 0U;
  s_usb_cmd_parse_length = 0U;
  s_usb_cmd_parse_payload_index = 0U;
  s_usb_cmd_parse_crc_rx = 0U;
}

static void usb_cmd_queue_reset(void)
{
  s_usb_cmd_queue_head = 0U;
  s_usb_cmd_queue_tail = 0U;
}

static uint16_t usb_cmd_crc16_ccitt(const uint8_t* data, uint32_t length)
{
  uint16_t crc = 0xFFFFU;

  for (uint32_t i = 0U; i < length; i++)
  {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t bit = 0U; bit < 8U; bit++)
    {
      if ((crc & 0x8000U) != 0U)
      {
        crc = (uint16_t)((crc << 1) ^ 0x1021U);
      }
      else
      {
        crc <<= 1;
      }
    }
  }

  return crc;
}

static void usb_cmd_queue_push(uint8_t command_id,
                               uint8_t sequence,
                               const uint8_t* payload,
                               uint8_t payload_len)
{
  uint8_t next_head = (uint8_t)((s_usb_cmd_queue_head + 1U) % USB_CMD_QUEUE_LEN);

  if (next_head == s_usb_cmd_queue_tail)
  {
    return;
  }

  s_usb_cmd_queue[s_usb_cmd_queue_head].command_id = command_id;
  s_usb_cmd_queue[s_usb_cmd_queue_head].sequence = sequence;
  s_usb_cmd_queue[s_usb_cmd_queue_head].payload_len = payload_len;

  if ((payload != NULL) && (payload_len > 0U))
  {
    (void)memcpy(s_usb_cmd_queue[s_usb_cmd_queue_head].payload, payload, payload_len);
  }

  s_usb_cmd_queue_head = next_head;
}

static void usb_cmd_parse_feed_byte(uint8_t byte)
{
  switch (s_usb_cmd_parse_state)
  {
    case USB_CMD_PARSE_WAIT_MAGIC_0:
      if (byte == USB_CMD_MAGIC_0)
      {
        s_usb_cmd_parse_state = USB_CMD_PARSE_WAIT_MAGIC_1;
      }
      break;

    case USB_CMD_PARSE_WAIT_MAGIC_1:
      if (byte == USB_CMD_MAGIC_1)
      {
        s_usb_cmd_parse_state = USB_CMD_PARSE_VERSION;
      }
      else if (byte != USB_CMD_MAGIC_0)
      {
        s_usb_cmd_parse_state = USB_CMD_PARSE_WAIT_MAGIC_0;
      }
      break;

    case USB_CMD_PARSE_VERSION:
      s_usb_cmd_parse_version = byte;
      s_usb_cmd_parse_state = USB_CMD_PARSE_COMMAND;
      break;

    case USB_CMD_PARSE_COMMAND:
      s_usb_cmd_parse_command = byte;
      s_usb_cmd_parse_state = USB_CMD_PARSE_SEQUENCE;
      break;

    case USB_CMD_PARSE_SEQUENCE:
      s_usb_cmd_parse_sequence = byte;
      s_usb_cmd_parse_state = USB_CMD_PARSE_LENGTH;
      break;

    case USB_CMD_PARSE_LENGTH:
      s_usb_cmd_parse_length = byte;
      s_usb_cmd_parse_payload_index = 0U;

      if (s_usb_cmd_parse_length > USB_CMD_MAX_PAYLOAD)
      {
        usb_cmd_parse_reset();
      }
      else if (s_usb_cmd_parse_length == 0U)
      {
        s_usb_cmd_parse_state = USB_CMD_PARSE_CRC_LO;
      }
      else
      {
        s_usb_cmd_parse_state = USB_CMD_PARSE_PAYLOAD;
      }
      break;

    case USB_CMD_PARSE_PAYLOAD:
      s_usb_cmd_parse_payload[s_usb_cmd_parse_payload_index++] = byte;
      if (s_usb_cmd_parse_payload_index >= s_usb_cmd_parse_length)
      {
        s_usb_cmd_parse_state = USB_CMD_PARSE_CRC_LO;
      }
      break;

    case USB_CMD_PARSE_CRC_LO:
      s_usb_cmd_parse_crc_rx = byte;
      s_usb_cmd_parse_state = USB_CMD_PARSE_CRC_HI;
      break;

    case USB_CMD_PARSE_CRC_HI:
    {
      uint8_t header_and_payload[6U + USB_CMD_MAX_PAYLOAD];
      uint16_t crc_calculated;
      uint16_t crc_received;

      s_usb_cmd_parse_crc_rx |= (uint16_t)((uint16_t)byte << 8);

      header_and_payload[0] = USB_CMD_MAGIC_0;
      header_and_payload[1] = USB_CMD_MAGIC_1;
      header_and_payload[2] = s_usb_cmd_parse_version;
      header_and_payload[3] = s_usb_cmd_parse_command;
      header_and_payload[4] = s_usb_cmd_parse_sequence;
      header_and_payload[5] = s_usb_cmd_parse_length;

      if (s_usb_cmd_parse_length > 0U)
      {
        (void)memcpy(&header_and_payload[6], s_usb_cmd_parse_payload, s_usb_cmd_parse_length);
      }

      crc_calculated = usb_cmd_crc16_ccitt(header_and_payload, (uint32_t)(6U + s_usb_cmd_parse_length));
      crc_received = s_usb_cmd_parse_crc_rx;

      if ((s_usb_cmd_parse_version == USB_CMD_VERSION_1) && (crc_calculated == crc_received))
      {
        usb_cmd_queue_push(s_usb_cmd_parse_command,
                           s_usb_cmd_parse_sequence,
                           s_usb_cmd_parse_payload,
                           s_usb_cmd_parse_length);
      }

      usb_cmd_parse_reset();
      break;
    }

    default:
      usb_cmd_parse_reset();
      break;
  }
}

uint8_t usb_cmd_try_dequeue(usb_cmd_packet_t* out_packet)
{
  uint8_t tail;
  uint8_t head;

  if (out_packet == NULL)
  {
    return 0U;
  }

  tail = s_usb_cmd_queue_tail;
  head = s_usb_cmd_queue_head;

  if ((tail >= USB_CMD_QUEUE_LEN) || (head >= USB_CMD_QUEUE_LEN))
  {
    usb_cmd_queue_reset();
    return 0U;
  }

  if (tail == head)
  {
    return 0U;
  }

  out_packet->command_id = s_usb_cmd_queue[tail].command_id;
  out_packet->sequence = s_usb_cmd_queue[tail].sequence;
  out_packet->payload_len = s_usb_cmd_queue[tail].payload_len;

  if (out_packet->payload_len > USB_CMD_MAX_PAYLOAD)
  {
    usb_cmd_queue_reset();
    return 0U;
  }

  for (uint8_t i = 0U; i < out_packet->payload_len; i++)
  {
    out_packet->payload[i] = s_usb_cmd_queue[tail].payload[i];
  }

  s_usb_cmd_queue_tail = (uint8_t)((tail + 1U) % USB_CMD_QUEUE_LEN);
  return 1U;
}

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
