/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "usbd_cdc_if.h"
#include "usbd_core.h"
#include "usbd_desc.h"

#include "core_cm7.h"

#include "epc660.h"
#include "epc660_platform.h"
#include "epc660_config.h"
#include "epc660_reg.h"
#include "usb_stream.h"
#include "capture_controller.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
  CAPTURE_4DCS = 0,
  CAPTURE_2DCS,
  CAPTURE_AMPLITUDE,
  CAPTURE_GRAYSCALE
} capture_types;

typedef enum
{
  EPC660_POWER_ON = 0,
  EPC660_POWER_OFF
} epc660_power_state_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IMG_WIDTH 320
#define IMG_HEIGHT 240
#define SINGLE_FRAME_SIZE (IMG_WIDTH * IMG_HEIGHT)
#define NUM_FRAMES 4

#define PHASE_TO_U16_SCALE (10430.378f)
#define BUTTON_DEBOUNCE_MS 150U
#define POWER_OFF_BLINK_MS 100U
#define CAPTURE_WAIT_TIMEOUT_MS 2000U

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;

I2C_HandleTypeDef hi2c2;

/* USER CODE BEGIN PV */

/******************************* DEBUGGING TRIGGERS *******************************/
static volatile uint8_t trigger_debug = 0;
static volatile uint8_t trigger_amplitude = 0;
/******************************* DEBUGGING TRIGGERS *******************************/

extern uint8_t UserTxBufferHS[]; /* Access the buffer we moved to Safe RAM */
extern USBD_HandleTypeDef hUsbDeviceHS;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

static uint16_t quad_frame_buffer[IMG_WIDTH*IMG_HEIGHT*NUM_FRAMES] __attribute__((aligned(32)));

// One buffer to reference frame angles
static uint16_t reference_angle_buffer[SINGLE_FRAME_SIZE] __attribute__((aligned(32)));

// One buffer for current frame angles
static uint16_t current_angle_buffer[SINGLE_FRAME_SIZE] __attribute__((aligned(32)));

// Active buffer to transmit over USB (processed output or raw grayscale frame)
static uint16_t* current_output_buffer = current_angle_buffer;

// A "flag" to signal when a frame is ready to be sent
static volatile uint8_t frame_ready = 0;

// Pointers to the start of each frame
static uint16_t* p_frame1 = &quad_frame_buffer[SINGLE_FRAME_SIZE * 0];
static uint16_t* p_frame2 = &quad_frame_buffer[SINGLE_FRAME_SIZE * 1];
static uint16_t* p_frame3 = &quad_frame_buffer[SINGLE_FRAME_SIZE * 2];
static uint16_t* p_frame4 = &quad_frame_buffer[SINGLE_FRAME_SIZE * 3];

static const uint32_t k_dcmi_dma_length = (SINGLE_FRAME_SIZE * NUM_FRAMES) / 2;
static const uint32_t k_dcmi_dma_length_grayscale = SINGLE_FRAME_SIZE / 2;
static uint32_t current_dcmi_dma_length = k_dcmi_dma_length;


static volatile uint8_t all_frames_ready = 0;
static volatile capture_types current_capture_type = CAPTURE_4DCS;
volatile uint8_t g_usb_capture_request_pending = 0;
volatile uint8_t g_usb_status_request_pending = 0;
static volatile uint8_t dcmi_error_flag = 0;
static volatile uint8_t reset_epc660_flag = 0;
static volatile epc660_power_state_t epc660_power_state = EPC660_POWER_ON;
static volatile uint8_t epc660_power_toggle_request = 0;
static volatile uint32_t button_last_press_ms = 0;
static volatile uint32_t saturated_pixel_count = 0;

static volatile uint32_t dbg_stats_valid_count = 0;
static volatile uint32_t dbg_stats_saturated_count = 0;
static volatile float dbg_mean_dcs0 = 0.0f;
static volatile float dbg_mean_dcs1 = 0.0f;
static volatile float dbg_mean_dcs2 = 0.0f;
static volatile float dbg_mean_dcs3 = 0.0f;
static volatile float dbg_mean_abs_i = 0.0f;
static volatile float dbg_mean_abs_q = 0.0f;

static volatile capture_sm_state_t capture_sm_state = CAPTURE_SM_IDLE;
static volatile uint8_t capture_request_active = 0;
static volatile uint8_t capture_recovery_attempted = 0;
static volatile uint32_t capture_wait_start_ms = 0;

static volatile uint32_t shutter_count = 0;
static volatile uint32_t start_dma_ok_count = 0;
static volatile uint32_t start_dma_fail_count = 0;
static volatile uint32_t frame_cb_count = 0;
static volatile uint32_t frame_timeout_count = 0;
static volatile uint32_t tx_frame_ok_count = 0;
static volatile uint32_t tx_frame_abort_count = 0;
static volatile uint32_t power_cycle_count = 0;
static volatile uint32_t pause_resume_count = 0;

extern USBD_HandleTypeDef hUsbDeviceHS;

// Some timer variables to be used with DWT
static volatile uint32_t tDCMI_start, tDCMI_end;

// Watchdog counter
volatile uint32_t g_last_feed_time = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C2_Init(void);
static void MX_DCMI_Init(void);
/* USER CODE BEGIN PFP */

static uint8_t process_and_transmit_data(void);
static void handle_dcmi_error(void);
static epc_status_t apply_capture_preset(void);
static void handle_epc660_reset(void);
static void handle_epc660_power_toggle_request(void);
static epc_status_t power_up_epc660_and_resume_capture(void);
static void power_down_epc660(void);
static void usb_stop_cleanly(void);
static void usb_start_nonfatal(void);
static void dcmi_stop_abort_and_clear(void);
static void reset_capture_runtime_flags(void);
static uint8_t start_capture_once(void);
static void process_usb_command_queue(void);
static uint8_t send_usb_command_response(uint8_t sequence,
                                         uint8_t command,
                                         uint8_t status,
                                         const uint8_t* payload,
                                         uint8_t payload_len);
static uint16_t usb_cmd_crc16_ccitt(const uint8_t* data, uint32_t length);
static void send_debug_status_line(void);
static void run_capture_state_machine(void);
void calculate_depth_simple(void);
void calculate_amplitude_simple(void);
void calculate_grayscale_simple(void);

epc_status_t default_preset_4DCS();
epc_status_t default_preset_grayscale();
epc_status_t testing_preset();
epc_status_t testing_preset2();

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C2_Init();
  //MX_USB_DEVICE_Init();
  MX_DCMI_Init();
  /* USER CODE BEGIN 2 */

  // This is a replacement of DWT_init
  epc_timer_init();

  // TODO: Should be removed from this section
  uint32_t loop_tick = 0;
  uint32_t frame_timeout_ms = 5000;

  epc_status_t init_status = epc660_power_up();
  if (init_status == EPC_OK)
  {
    init_status = epc660_init();
  }

  if (init_status != EPC_OK)
  {
    power_down_epc660();
  }

  usb_start_nonfatal();

  all_frames_ready = 0;

  if (epc660_power_state == EPC660_POWER_ON)
  {
    epc_status_t preset_status = apply_capture_preset();

    if (preset_status != EPC_OK)
    {
      power_down_epc660();
    }
    else
    {
      reset_capture_runtime_flags();
      capture_sm_state = CAPTURE_SM_IDLE;
    }
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // Feed the dog	TODO: Currently disabled, need to re-enable later
    g_last_feed_time = HAL_GetTick();

    if (epc660_power_toggle_request == 1)
    {
      handle_epc660_power_toggle_request();
    }

    process_usb_command_queue();

    if (g_usb_status_request_pending != 0U)
    {
      send_debug_status_line();
      g_usb_status_request_pending = 0U;
    }

    if (epc660_power_state == EPC660_POWER_OFF)
    {
      HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
      HAL_Delay(POWER_OFF_BLINK_MS);
      continue;
    }

	  // Blinking light to make sure main loop is still running
	  if (HAL_GetTick() - loop_tick > frame_timeout_ms)
	  {
		  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
		  loop_tick = HAL_GetTick();
	  }

	  // If user sends configuration information
	  // If time to do temperature calibration

    run_capture_state_machine();

    if (reset_epc660_flag == 1)
    {
      handle_epc660_reset();
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // Feed the dog	TODO: Currently disabled, need to re-enable later
    if (trigger_debug == 1)
    {
      default_preset_4DCS();
      default_preset_grayscale();
      testing_preset2();
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /*AXI clock gating */
  RCC->CKGAENR = 0xE003FFFF;

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 112;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DCMI Initialization Function
  * @param None
  * @retval None
  */
static void MX_DCMI_Init(void)
{

  /* USER CODE BEGIN DCMI_Init 0 */

  /* USER CODE END DCMI_Init 0 */

  /* USER CODE BEGIN DCMI_Init 1 */

  /* USER CODE END DCMI_Init 1 */
  hdcmi.Instance = DCMI;
  hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
  hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_FALLING;
  hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
  hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_HIGH;
  hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
  hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_12B;
  hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
  hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
  hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
  hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
  hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;
  if (HAL_DCMI_Init(&hdcmi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DCMI_Init 2 */

  /* USER CODE END DCMI_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x20B0CCFF;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CC_RESET_GPIO_Port, CC_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CC_SHUTTER_GPIO_Port, CC_SHUTTER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, ENABLE_5V_10V_Pin|ENABLE_1V8_3V3_Pin|ENABLE_NEG10V_Pin|LED1_Pin
                          |ENABLE_15V_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED2_Pin */
  GPIO_InitStruct.Pin = LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CC_RESET_Pin */
  GPIO_InitStruct.Pin = CC_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CC_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CC_SHUTTER_Pin */
  GPIO_InitStruct.Pin = CC_SHUTTER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CC_SHUTTER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ENABLE_5V_10V_Pin ENABLE_1V8_3V3_Pin ENABLE_NEG10V_Pin LED1_Pin
                           ENABLE_15V_Pin */
  GPIO_InitStruct.Pin = ENABLE_5V_10V_Pin|ENABLE_1V8_3V3_Pin|ENABLE_NEG10V_Pin|LED1_Pin
                          |ENABLE_15V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : LED3_Pin */
  GPIO_InitStruct.Pin = LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BUTTON_Pin */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(BUTTON_EXTI_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(BUTTON_EXTI_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/************************************************* USER DEFINED FUNCTIONS ********************************************/
/************************************************* USER DEFINED FUNCTIONS ********************************************/
/************************************************* USER DEFINED FUNCTIONS ********************************************/
static uint8_t process_and_transmit_data(void)
{
  uint8_t frame_processed = 0;
  uint8_t transmit_raw_quad = 0;
  uint8_t tx_ok = 0;

  all_frames_ready = 0;
  SCB_InvalidateDCache_by_Addr((uint32_t*)quad_frame_buffer, current_dcmi_dma_length * sizeof(uint32_t));

  switch (current_capture_type)
  {
    case CAPTURE_AMPLITUDE:
      calculate_amplitude_simple();
      frame_processed = 1;
      break;

    case CAPTURE_4DCS:
      transmit_raw_quad = 1;
      break;

    case CAPTURE_GRAYSCALE:
      calculate_grayscale_simple();
      frame_processed = 1;
      break;

    case CAPTURE_2DCS:
    default:
      break;
  }

  if (transmit_raw_quad != 0)
  {
    SCB_CleanDCache_by_Addr((uint32_t*)quad_frame_buffer, SINGLE_FRAME_SIZE * NUM_FRAMES * 2);
    tx_ok = usb_stream_transmit_quad_frame(quad_frame_buffer,
                         SINGLE_FRAME_SIZE,
                         NUM_FRAMES);
  }
  else
  {
    if (frame_processed != 0)
    {
      SCB_CleanDCache_by_Addr((uint32_t*)current_output_buffer, SINGLE_FRAME_SIZE * 2);
    }

    tx_ok = usb_stream_transmit_frame(current_output_buffer,
                      SINGLE_FRAME_SIZE);
  }

  return tx_ok;
}

static epc_status_t apply_capture_preset(void)
{
  epc_status_t preset_status = EPC_OK;

  switch (current_capture_type)
  {
    case CAPTURE_4DCS:
      preset_status = default_preset_4DCS();
      current_dcmi_dma_length = k_dcmi_dma_length;
      current_output_buffer = current_angle_buffer;
      break;

    case CAPTURE_2DCS:
      preset_status = testing_preset();
      current_dcmi_dma_length = k_dcmi_dma_length;
      current_output_buffer = current_angle_buffer;
      break;

    case CAPTURE_AMPLITUDE:
      preset_status = testing_preset();
      current_dcmi_dma_length = k_dcmi_dma_length;
      current_output_buffer = current_angle_buffer;
      break;

    case CAPTURE_GRAYSCALE:
      preset_status = default_preset_grayscale();
      current_dcmi_dma_length = k_dcmi_dma_length_grayscale;
      current_output_buffer = current_angle_buffer;
      break;

    default:
      preset_status = testing_preset();
      current_dcmi_dma_length = k_dcmi_dma_length;
      current_output_buffer = current_angle_buffer;
      break;
  }

  return preset_status;
}

static void handle_epc660_reset(void)
{
  reset_epc660_flag = 0;
  reset_capture_runtime_flags();
  dcmi_stop_abort_and_clear();

  epc_reset_pin_set(0);
  epc_delay_ms(1);
  epc_reset_pin_set(1);
  epc_delay_ms(2);

  if (epc660_init() != EPC_OK)
  {
    power_down_epc660();
    return;
  }

  if (apply_capture_preset() != EPC_OK)
  {
    power_down_epc660();
    return;
  }

  capture_sm_state = CAPTURE_SM_IDLE;
}

static void handle_epc660_power_toggle_request(void)
{
  epc660_power_toggle_request = 0;
  pause_resume_count++;
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

  if (epc660_power_state == EPC660_POWER_ON)
  {
    power_down_epc660();
  }
  else if (power_up_epc660_and_resume_capture() != EPC_OK)
  {
    power_down_epc660();
  }
}

static void power_down_epc660(void)
{
  reset_capture_runtime_flags();
  dcmi_stop_abort_and_clear();

  __HAL_DCMI_DISABLE_IT(&hdcmi,
                        DCMI_IT_FRAME |
                        DCMI_IT_OVF |
                        DCMI_IT_ERR |
                        DCMI_IT_VSYNC |
                        DCMI_IT_LINE);

  __HAL_DCMI_CLEAR_FLAG(&hdcmi,
                        DCMI_FLAG_FRAMERI |
                        DCMI_FLAG_OVRRI |
                        DCMI_FLAG_ERRRI |
                        DCMI_FLAG_VSYNCRI |
                        DCMI_FLAG_LINERI);

  hdcmi.ErrorCode = HAL_DCMI_ERROR_NONE;
  HAL_NVIC_DisableIRQ(DMA1_Stream0_IRQn);

  usb_stop_cleanly();

  epc660_power_down();
  epc660_power_state = EPC660_POWER_OFF;
}

static epc_status_t power_up_epc660_and_resume_capture(void)
{
  uint8_t was_powered_off = (epc660_power_state == EPC660_POWER_OFF) ? 1U : 0U;

  if (epc660_power_up() != EPC_OK)
  {
    power_down_epc660();
    return EPC_ERR;
  }

  // Bring USB back after EPC rails are enabled. If host is absent, keep running.
  usb_start_nonfatal();

  if (epc660_init() != EPC_OK)
  {
    power_down_epc660();
    return EPC_ERR;
  }

  if (apply_capture_preset() != EPC_OK)
  {
    power_down_epc660();
    return EPC_ERR;
  }
  epc660_power_state = EPC660_POWER_ON;
  if (was_powered_off != 0U)
  {
    power_cycle_count++;
  }
  reset_capture_runtime_flags();
  capture_sm_state = CAPTURE_SM_IDLE;

  __HAL_DCMI_CLEAR_FLAG(&hdcmi,
                        DCMI_FLAG_FRAMERI |
                        DCMI_FLAG_OVRRI |
                        DCMI_FLAG_ERRRI |
                        DCMI_FLAG_VSYNCRI |
                        DCMI_FLAG_LINERI);

  hdcmi.ErrorCode = HAL_DCMI_ERROR_NONE;

  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  __HAL_DCMI_ENABLE_IT(&hdcmi,
                       DCMI_IT_FRAME |
                       DCMI_IT_OVF |
                       DCMI_IT_ERR |
                       DCMI_IT_VSYNC |
                       DCMI_IT_LINE);

  return EPC_OK;
}

static void usb_stop_cleanly(void)
{
  // If USB stack was never initialized, pData is NULL and LL stop/deinit would hardfault.
  if (hUsbDeviceHS.pData != NULL)
  {
    (void)USBD_Stop(&hUsbDeviceHS);
    (void)USBD_DeInit(&hUsbDeviceHS);
    hUsbDeviceHS.pData = NULL;
  }

  HAL_NVIC_DisableIRQ(OTG_HS_IRQn);
  NVIC_ClearPendingIRQ(OTG_HS_IRQn);
}

static void usb_start_nonfatal(void)
{
  if (hUsbDeviceHS.dev_state == USBD_STATE_CONFIGURED)
  {
    return;
  }

  usb_stop_cleanly();
  HAL_NVIC_EnableIRQ(OTG_HS_IRQn);

  if (USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK)
  {
    return;
  }

  if (USBD_RegisterClass(&hUsbDeviceHS, &USBD_CDC) != USBD_OK)
  {
    (void)USBD_DeInit(&hUsbDeviceHS);
    return;
  }

  if (USBD_CDC_RegisterInterface(&hUsbDeviceHS, &USBD_Interface_fops_HS) != USBD_OK)
  {
    (void)USBD_DeInit(&hUsbDeviceHS);
    return;
  }

  if (USBD_Start(&hUsbDeviceHS) != USBD_OK)
  {
    (void)USBD_DeInit(&hUsbDeviceHS);
    return;
  }

  HAL_PWREx_EnableUSBVoltageDetector();
}

static void dcmi_stop_abort_and_clear(void)
{
  HAL_DCMI_Stop(&hdcmi);

  if (hdcmi.DMA_Handle != NULL)
  {
    (void)HAL_DMA_Abort(hdcmi.DMA_Handle);
  }

  __HAL_DCMI_CLEAR_FLAG(&hdcmi,
                        DCMI_FLAG_FRAMERI |
                        DCMI_FLAG_OVRRI |
                        DCMI_FLAG_ERRRI |
                        DCMI_FLAG_VSYNCRI |
                        DCMI_FLAG_LINERI);

  hdcmi.ErrorCode = HAL_DCMI_ERROR_NONE;
}

static void reset_capture_runtime_flags(void)
{
  all_frames_ready = 0;
  frame_ready = 0;
  dcmi_error_flag = 0;
  g_usb_capture_request_pending = 0;
  capture_request_active = 0;
  capture_recovery_attempted = 0;
  capture_wait_start_ms = 0;
}

static uint8_t start_capture_once(void)
{
  all_frames_ready = 0;
  frame_ready = 0;
  dcmi_error_flag = 0;

  SCB_InvalidateDCache_by_Addr((uint32_t*)quad_frame_buffer, current_dcmi_dma_length * sizeof(uint32_t));

  if (HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)quad_frame_buffer, current_dcmi_dma_length) != HAL_OK)
  {
    start_dma_fail_count++;
    return 0;
  }

  start_dma_ok_count++;
  epc660_trigger_hw_shutter();
  shutter_count++;
  capture_wait_start_ms = HAL_GetTick();
  return 1;
}

static void process_usb_command_queue(void)
{
  usb_cmd_packet_t packet;

  while (usb_cmd_try_dequeue(&packet) != 0U)
  {
    switch (packet.command_id)
    {
      case USB_CMD_GET_FRAME:
        g_usb_capture_request_pending = 1U;
        break;

      case USB_CMD_GET_STATUS:
        g_usb_status_request_pending = 1U;
        break;

      case USB_CMD_SET_MOD_DIVIDER:
      {
        epc_status_t status;

        if (packet.payload_len != 1U)
        {
          (void)send_usb_command_response(packet.sequence,
                                          packet.command_id,
                                          USB_CMD_STATUS_BAD_LENGTH,
                                          NULL,
                                          0U);
          break;
        }

        if (epc660_power_state != EPC660_POWER_ON)
        {
          (void)send_usb_command_response(packet.sequence,
                                          packet.command_id,
                                          USB_CMD_STATUS_REJECTED,
                                          NULL,
                                          0U);
          break;
        }

        status = epc_set_modulation_divider(packet.payload[0]);
        (void)send_usb_command_response(packet.sequence,
                                        packet.command_id,
                                        (status == EPC_OK) ? USB_CMD_STATUS_OK : USB_CMD_STATUS_DEVICE_ERROR,
                                        NULL,
                                        0U);
        break;
      }

      case USB_CMD_SET_INTEGRATION_RAW:
      {
        uint8_t integration_slot;
        uint16_t integration_raw;
        epc_status_t status;

        if (packet.payload_len != 3U)
        {
          (void)send_usb_command_response(packet.sequence,
                                          packet.command_id,
                                          USB_CMD_STATUS_BAD_LENGTH,
                                          NULL,
                                          0U);
          break;
        }

        if (epc660_power_state != EPC660_POWER_ON)
        {
          (void)send_usb_command_response(packet.sequence,
                                          packet.command_id,
                                          USB_CMD_STATUS_REJECTED,
                                          NULL,
                                          0U);
          break;
        }

        integration_slot = packet.payload[0];
        integration_raw = (uint16_t)packet.payload[1] | ((uint16_t)packet.payload[2] << 8);

        status = epc_set_integration_time_raw(integration_slot, integration_raw);
        (void)send_usb_command_response(packet.sequence,
                                        packet.command_id,
                                        (status == EPC_OK) ? USB_CMD_STATUS_OK : USB_CMD_STATUS_DEVICE_ERROR,
                                        NULL,
                                        0U);
        break;
      }

      default:
        (void)send_usb_command_response(packet.sequence,
                                        packet.command_id,
                                        USB_CMD_STATUS_UNSUPPORTED,
                                        NULL,
                                        0U);
        break;
    }
  }
}

static uint8_t send_usb_command_response(uint8_t sequence,
                                         uint8_t command,
                                         uint8_t status,
                                         const uint8_t* payload,
                                         uint8_t payload_len)
{
  uint8_t frame[6U + USB_CMD_MAX_PAYLOAD + 2U];
  uint8_t response_payload_len;
  uint16_t crc;
  uint8_t frame_len;

  if (payload_len > (USB_CMD_MAX_PAYLOAD - 2U))
  {
    return 0U;
  }

  response_payload_len = (uint8_t)(payload_len + 2U);

  frame[0] = USB_CMD_MAGIC_0;
  frame[1] = USB_CMD_MAGIC_1;
  frame[2] = USB_CMD_VERSION_1;
  frame[3] = USB_CMD_RESPONSE;
  frame[4] = sequence;
  frame[5] = response_payload_len;
  frame[6] = command;
  frame[7] = status;

  if ((payload != NULL) && (payload_len > 0U))
  {
    (void)memcpy(&frame[8], payload, payload_len);
  }

  crc = usb_cmd_crc16_ccitt(frame, (uint32_t)(6U + response_payload_len));
  frame[6U + response_payload_len] = (uint8_t)(crc & 0xFFU);
  frame[7U + response_payload_len] = (uint8_t)((crc >> 8) & 0xFFU);

  frame_len = (uint8_t)(8U + response_payload_len);
  return usb_stream_transmit_blocking(frame, frame_len);
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

static void send_debug_status_line(void)
{
  char line[224];
  int n;
  uint8_t mod_div = 0xFF;

  if (hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED)
  {
    return;
  }

  if (epc660_power_state == EPC660_POWER_ON)
  {
    (void)epc_i2c_read(EPC_REG_MOD_DIV, &mod_div, EPC_DIRECT);
  }

  n = snprintf(line,
               sizeof(line),
               "ST s=%s sh=%lu ok=%lu sf=%lu cb=%lu to=%lu tx=%lu ta=%lu pc=%lu pr=%lu md=%u\r\n",
               capture_controller_state_to_string(capture_sm_state),
               (unsigned long)shutter_count,
               (unsigned long)start_dma_ok_count,
               (unsigned long)start_dma_fail_count,
               (unsigned long)frame_cb_count,
               (unsigned long)frame_timeout_count,
               (unsigned long)tx_frame_ok_count,
               (unsigned long)tx_frame_abort_count,
               (unsigned long)power_cycle_count,
               (unsigned long)pause_resume_count,
               (unsigned int)mod_div);

  if (n <= 0)
  {
    return;
  }

  if (n > (int)sizeof(line))
  {
    n = (int)sizeof(line);
  }

  (void)usb_stream_transmit_blocking((uint8_t*)line, (uint16_t)n);
}

static void run_capture_state_machine(void)
{
  capture_controller_ctx_t ctx = {
    .capture_sm_state = &capture_sm_state,
    .usb_capture_request_pending = &g_usb_capture_request_pending,
    .capture_request_active = &capture_request_active,
    .capture_recovery_attempted = &capture_recovery_attempted,
    .dcmi_error_flag = &dcmi_error_flag,
    .all_frames_ready = &all_frames_ready,
    .capture_wait_start_ms = &capture_wait_start_ms,
    .frame_timeout_count = &frame_timeout_count,
    .tx_frame_ok_count = &tx_frame_ok_count,
    .tx_frame_abort_count = &tx_frame_abort_count,
    .capture_wait_timeout_ms = CAPTURE_WAIT_TIMEOUT_MS
  };

  const capture_controller_ops_t ops = {
    .start_capture_once = start_capture_once,
    .process_and_transmit_data = process_and_transmit_data,
    .handle_dcmi_error = handle_dcmi_error,
    .dcmi_stop_abort_and_clear = dcmi_stop_abort_and_clear,
    .get_tick_ms = HAL_GetTick
  };

  capture_controller_run(&ctx, &ops);
}

static void handle_dcmi_error(void)
{
  // Keep this lightweight and deterministic in request mode.
  dcmi_error_flag = 0;
  dcmi_stop_abort_and_clear();
}

void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  if (hdcmi->Instance == DCMI)
  {
    frame_cb_count++;
    frame_ready = 1;
    all_frames_ready = 1;
  }
}
void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{
    // Make sure this interrupt came from our specific DCMI peripheral
    if (hdcmi->Instance == DCMI)
    {
        uint32_t error_code = hdcmi->ErrorCode;

        // 1. Overrun Error (Extremely common!)
        // This means the camera sent data faster than the DMA could push it into RAM.
        if (error_code & HAL_DCMI_ERROR_OVR)
        {
            // TODO: Handle overrun (e.g., print warning, flash RED LED)
        }

        // 2. Synchronization Error
        // This means the DCMI detected a glitch or unexpected VSYNC/HSYNC pulse.
        if (error_code & HAL_DCMI_ERROR_SYNC)
        {
            // TODO: Handle sync error
        }

        // 3. Timeout Error
        // DCMI Timeout
        if (error_code & HAL_DCMI_ERROR_TIMEOUT)
        {
            // TODO: Handle line error
        }

        dcmi_error_flag = 1;
    }
}

void calculate_depth_simple(void)
{
  uint32_t saturated_count = 0;
  for (uint32_t i = 0; i < SINGLE_FRAME_SIZE; i++)
    {
		uint16_t raw1 = (p_frame1[i] & 0x0FFF);
		uint16_t raw2 = (p_frame2[i] & 0x0FFF);
		uint16_t raw3 = (p_frame3[i] & 0x0FFF);
		uint16_t raw4 = (p_frame4[i] & 0x0FFF);

		if ((raw1 == 0x0FFF) || (raw2 == 0x0FFF) || (raw3 == 0x0FFF) || (raw4 == 0x0FFF))
		{
			current_angle_buffer[i] = 0xFFFF;
			saturated_count++;
			continue;
		}

		int16_t i1 = (int16_t)(raw1 - 2048);
		int16_t i2 = (int16_t)(raw2 - 2048);
		int16_t i3 = (int16_t)(raw3 - 2048);
		int16_t i4 = (int16_t)(raw4 - 2048);

        int16_t Q = i4 - i2;
        int16_t I = i3 - i1;

        float phase = atan2f((float)Q, (float)I);
        if (phase < 0) phase += 2.0f * M_PI;

        current_angle_buffer[i] = (uint16_t)(phase * PHASE_TO_U16_SCALE);
    }

  saturated_pixel_count = saturated_count;
}

void calculate_amplitude_simple(void)
{
  uint32_t saturated_count = 0;
  // For stats, delete later
  uint64_t sum_dcs0 = 0;
  uint64_t sum_dcs1 = 0;
  uint64_t sum_dcs2 = 0;
  uint64_t sum_dcs3 = 0;
  uint64_t sum_abs_i = 0;
  uint64_t sum_abs_q = 0;
  uint32_t valid_count = 0;
  // For stats, delete later
  for (uint32_t i = 0; i < SINGLE_FRAME_SIZE; i++)
  {
  uint16_t raw1 = (p_frame1[i] & 0x0FFF);
  uint16_t raw2 = (p_frame2[i] & 0x0FFF);
  uint16_t raw3 = (p_frame3[i] & 0x0FFF);
  uint16_t raw4 = (p_frame4[i] & 0x0FFF);

  if ((raw1 == 0x0FFF) || (raw2 == 0x0FFF) || (raw3 == 0x0FFF) || (raw4 == 0x0FFF))
  {
    current_angle_buffer[i] = 0xFFFF;
    saturated_count++;
    continue;
  }

  int16_t i1 = (int16_t)(raw1 - 2048);
  int16_t i2 = (int16_t)(raw2 - 2048);
  int16_t i3 = (int16_t)(raw3 - 2048);
  int16_t i4 = (int16_t)(raw4 - 2048);

  int32_t q = (int32_t)i4 - (int32_t)i2;
  int32_t in_phase = (int32_t)i3 - (int32_t)i1;
  // For stats, delete later
  int32_t abs_i = (in_phase < 0) ? -in_phase : in_phase;
  int32_t abs_q = (q < 0) ? -q : q;

  sum_dcs0 += raw1;
  sum_dcs1 += raw2;
  sum_dcs2 += raw3;
  sum_dcs3 += raw4;
  sum_abs_i += (uint32_t)abs_i;
  sum_abs_q += (uint32_t)abs_q;
  valid_count++;
  // For stats, delete later
  float amplitude = 0.5f * sqrtf((float)(q * q + in_phase * in_phase));

  if (amplitude > 65534.0f)
  {
    amplitude = 65534.0f;
  }

  current_angle_buffer[i] = (uint16_t)(amplitude + 0.5f);
  }

  saturated_pixel_count = saturated_count;
  
  // For stats, delete later
  dbg_stats_valid_count = valid_count;
  dbg_stats_saturated_count = saturated_count;

  if (valid_count > 0)
  {
    float inv_valid = 1.0f / (float)valid_count;
    dbg_mean_dcs0 = (float)sum_dcs0 * inv_valid;
    dbg_mean_dcs1 = (float)sum_dcs1 * inv_valid;
    dbg_mean_dcs2 = (float)sum_dcs2 * inv_valid;
    dbg_mean_dcs3 = (float)sum_dcs3 * inv_valid;
    dbg_mean_abs_i = (float)sum_abs_i * inv_valid;
    dbg_mean_abs_q = (float)sum_abs_q * inv_valid;
  }
  else
  {
    dbg_mean_dcs0 = 0.0f;
    dbg_mean_dcs1 = 0.0f;
    dbg_mean_dcs2 = 0.0f;
    dbg_mean_dcs3 = 0.0f;
    dbg_mean_abs_i = 0.0f;
    dbg_mean_abs_q = 0.0f;
  }
  // For stats, delete later
}

void calculate_grayscale_simple(void)
{
  uint32_t saturated_count = 0;

  for (uint32_t i = 0; i < SINGLE_FRAME_SIZE; i++)
  {
    uint16_t raw = (p_frame1[i] & 0x0FFF);

    if (raw == 0x0FFF)
    {
      current_angle_buffer[i] = 0xFFFF;
      saturated_count++;
      continue;
    }

    current_angle_buffer[i] = raw;
  }

  saturated_pixel_count = saturated_count;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* Check if the interrupt came from BUTTON */
    if (GPIO_Pin == BUTTON_Pin)
    {
      uint32_t now = HAL_GetTick();

      if ((now - button_last_press_ms) >= BUTTON_DEBOUNCE_MS)
      {
        button_last_press_ms = now;
        epc660_power_toggle_request = 1;
      }
    }
}

/* EPC660 CONFIG PRESETS */
epc_status_t default_preset_4DCS()
{
	__NOP();
	volatile epc_status_t status;
	epc_set_measurement_mode(EPC_MODE_4DCS_TOF);
	epc_set_dclk_freq(EPC_DCLK_12MHZ);	// Slow Dclk speed
	epc_set_hsync_stretch(1);	//Hsync stretching
	status = epc_set_modulation_divider(0);	// Explicitly set 24MHz modulation
	if ((status = epc_set_integration_time_raw(2, 38399)) != EPC_OK) return status;	// This sets an integration time of 1.58us as per the datasheet
	epc_set_software_saturation_flag(1);

	return EPC_OK;
}
epc_status_t default_preset_grayscale()
{
	__NOP();
	volatile epc_status_t status;
	epc_set_measurement_mode(EPC_MODE_GRAYSCALE);
	epc_set_grayscale_mode(EPC_GS_MODULATED_LED);	// Illumination
	epc_set_dclk_freq(EPC_DCLK_12MHZ);	// Slow Dclk speed
	if (epc_i2c_write(0x3A, 0x30, EPC_DIRECT) != EPC_OK) return EPC_ERR;	// Grayscale setting thing
	epc_set_hsync_stretch(1);	//Hsync stretching
	status = epc_set_integration_time_raw(1, 4799);	// This sets an integration time of #### as per the datasheet

	return EPC_OK;
}
epc_status_t testing_preset()
{
	epc_status_t status;
	if ((status = epc_set_measurement_mode(EPC_MODE_4DCS_TOF)) != EPC_OK) return status;
	if ((status = epc_set_dclk_freq(EPC_DCLK_3MHZ)) != EPC_OK) return status;	// Slow Dclk speed
	if ((status = epc_set_hsync_stretch(1)) != EPC_OK) return status;	//Hsync stretching
	if ((status = epc_set_modulation_divider(0)) != EPC_OK) return status;	// Explicitly set 6MHz modulation
	if ((status = epc_set_integration_time_raw(1, 599)) != EPC_OK) return status;	// This sets an integration time of 1.58us as per the datasheet
	if ((status = epc_set_software_saturation_flag(1)) != EPC_OK) return status;

	return EPC_OK;
}
epc_status_t testing_preset2()
{
	epc_status_t status;
	if ((status = epc_set_measurement_mode(EPC_MODE_4DCS_TOF)) != EPC_OK) return status;

	if ((status = epc_set_dclk_freq(EPC_DCLK_24MHZ)) != EPC_OK) return status;	// Slow Dclk speed
	if ((status = epc_set_dclk_freq(EPC_DCLK_12MHZ)) != EPC_OK) return status;
	if ((status = epc_set_dclk_freq(EPC_DCLK_6MHZ)) != EPC_OK) return status;
	if ((status = epc_set_dclk_freq(EPC_DCLK_3MHZ)) != EPC_OK) return status;

	if ((status = epc_set_hsync_stretch(1)) != EPC_OK) return status;	//Hsync stretching
	if ((status = epc_set_hsync_stretch(0)) != EPC_OK) return status;

	if ((status = epc_set_modulation_divider(0)) != EPC_OK) return status;	// Explicitly set 6MHz modulation
	if ((status = epc_set_modulation_divider(1)) != EPC_OK) return status;
	if ((status = epc_set_modulation_divider(3)) != EPC_OK) return status;
	if ((status = epc_set_modulation_divider(7)) != EPC_OK) return status;
	if ((status = epc_set_modulation_divider(15)) != EPC_OK) return status;

	if ((status = epc_set_integration_time_raw(1, 75)) != EPC_OK) return status;	// This sets an integration time of 1.58us as per the datasheet
	if ((status = epc_set_integration_time_raw(1, 599)) != EPC_OK) return status;
	if ((status = epc_set_integration_time_raw(1, 4799)) != EPC_OK) return status;
	if ((status = epc_set_integration_time_raw(1, 9599)) != EPC_OK) return status;
	if ((status = epc_set_integration_time_raw(1, 19199)) != EPC_OK) return status;
	if ((status = epc_set_integration_time_raw(1, 38399)) != EPC_OK) return status;
	if ((status = epc_set_integration_time_raw(2, 38399)) != EPC_OK) return status;
	if ((status = epc_set_integration_time_raw(3, 38399)) != EPC_OK) return status;
	if ((status = epc_set_integration_time_raw(4, 38399)) != EPC_OK) return status;
	if ((status = epc_set_integration_time_raw(6, 38399)) != EPC_OK) return status;
	if ((status = epc_set_integration_time_raw(8, 38399)) != EPC_OK) return status;
	if ((status = epc_set_integration_time_raw(10, 38399)) != EPC_OK) return status;
	if ((status = epc_set_integration_time_raw(12, 38399)) != EPC_OK) return status;
	if ((status = epc_set_integration_time_raw(14, 38399)) != EPC_OK) return status;

	if ((status = epc_set_software_saturation_flag(1)) != EPC_OK) return status;
	if ((status = epc_set_software_saturation_flag(0)) != EPC_OK) return status;

	return EPC_OK;
}

/************************************************* USER DEFINED FUNCTIONS ********************************************/
/************************************************* USER DEFINED FUNCTIONS ********************************************/
/************************************************* USER DEFINED FUNCTIONS ********************************************/
/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_8KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  epc_emergency_power_down();
  __disable_irq();
  while (1)
  {
	HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);

	if (DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk)
	{
	  uint32_t start = DWT->CYCCNT;
	  /* ~100 ms delay: SystemCoreClock/10 cycles */
	  uint32_t wait = SystemCoreClock / 10;
	  while ((DWT->CYCCNT - start) < wait) { __NOP(); }
	}
	else
	{
	  for (volatile uint32_t i = 0; i < 2400000; ++i) { __NOP(); } /* adjust for speed */
	}
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
