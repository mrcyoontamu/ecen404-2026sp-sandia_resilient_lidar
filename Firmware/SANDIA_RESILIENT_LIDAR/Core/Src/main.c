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

#include "core_cm7.h"

#include "epc660.h"
#include "epc660_platform.h"
#include "epc660_config.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IMG_WIDTH 320
#define IMG_HEIGHT 240
#define SINGLE_FRAME_SIZE (IMG_WIDTH * IMG_HEIGHT)
#define NUM_FRAMES 4
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;

I2C_HandleTypeDef hi2c2;

/* USER CODE BEGIN PV */
extern uint8_t UserTxBufferHS[]; /* Access the buffer we moved to Safe RAM */
extern USBD_HandleTypeDef hUsbDeviceHS;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

uint16_t quad_frame_buffer[IMG_WIDTH*IMG_HEIGHT*NUM_FRAMES];

// One buffer to reference frame angles
uint16_t reference_angle_buffer[SINGLE_FRAME_SIZE];

// One buffer for current frame angles
uint16_t current_angle_buffer[SINGLE_FRAME_SIZE];

// A "flag" to signal when a frame is ready to be sent
volatile uint8_t frame_ready = 0;

// Pointers to the start of each frame
uint16_t* p_frame1 = &quad_frame_buffer[SINGLE_FRAME_SIZE * 0];
uint16_t* p_frame2 = &quad_frame_buffer[SINGLE_FRAME_SIZE * 1];
uint16_t* p_frame3 = &quad_frame_buffer[SINGLE_FRAME_SIZE * 2];
uint16_t* p_frame4 = &quad_frame_buffer[SINGLE_FRAME_SIZE * 3];

static const uint32_t k_dcmi_dma_length = (SINGLE_FRAME_SIZE * NUM_FRAMES) / 2;

volatile uint8_t all_frames_ready = 0;
volatile uint32_t frame_capture_count = 0;

extern USBD_HandleTypeDef hUsbDeviceHS;

// Some timer variables to be used with DWT
volatile uint32_t tDCMI_start, tDCMI_end;

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
void I2C_Scanner(void);

static void USB_Transmit_Blocking(uint8_t* Buf, uint16_t Len);
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
  MX_USB_DEVICE_Init();
  MX_DCMI_Init();
  /* USER CODE BEGIN 2 */

  // This is a replacement of DWT_init
  epc_timer_init();

  // TODO: Should be removed from this section
  uint32_t loop_tick = 0;
  uint32_t frame_timeout_ms = 5000;


  // Initialize epc660
  epc_status_t epcRET;

  // Disable USB interrupt during power-up to prevent spurious triggers
  // from EMI/voltage transients on the 10V/neg10V rails coupling into USB lines
  HAL_NVIC_DisableIRQ(OTG_HS_IRQn);

  if (epc660_power_up() != EPC_OK) Error_Handler();
  if (epc660_init() != EPC_OK) Error_Handler();
  //epc660_power_down();

  // Clear any pending USB interrupt flags that accumulated, then re-enable
  __HAL_PCD_CLEAR_FLAG(&hpcd_USB_OTG_HS, 0xFFFFFFFF);
  NVIC_ClearPendingIRQ(OTG_HS_IRQn);
  HAL_NVIC_EnableIRQ(OTG_HS_IRQn);

  all_frames_ready = 0;
  frame_capture_count = 0;

  const uint8_t FRAME_HEADER[4] = {0xAA, 0x55, 0xAA, 0x55};

  // Start capture (testing purposes)
  epc660_cfg_set_measurement_mode(EPC_MODE_GRAYSCALE);
  epc_i2c_write(0x3C, 0x26, EPC_DIRECT);	// No illumination
  epc660_cfg_set_dclk_freq(EPC_DCLK_12MHZ);	// Slow Dclk speed
  epc_i2c_write(0x91, 0x43, EPC_DIRECT);	//Hsync stretching

  // Read back some settings for sanity check
  uint8_t reg_readback = 0;
  epcRET = epc_i2c_read(0xCC, &reg_readback, EPC_DIRECT);

  HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)quad_frame_buffer, k_dcmi_dma_length);
  epc660_trigger_hw_shutter();
  //epc_i2c_write(0xA4, 0x01, EPC_DIRECT);	// Trigger SW Shutter

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // Blinking light to make sure main loop is still running
	  if (HAL_GetTick() - loop_tick > frame_timeout_ms)
	  {
		  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
		  loop_tick = HAL_GetTick();
		  epc660_trigger_hw_shutter();
	  }

	  if (all_frames_ready == 1)
	  {
		  all_frames_ready = 0;
		  SCB_InvalidateDCache_by_Addr((uint32_t*)quad_frame_buffer, SINGLE_FRAME_SIZE * NUM_FRAMES * 2);

	        while(hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED);

	        USB_Transmit_Blocking((uint8_t*)FRAME_HEADER, 4);

	        uint8_t* p_buffer = (uint8_t*)p_frame1;
	        uint32_t bytes_remaining = SINGLE_FRAME_SIZE;
	        uint16_t chunk_size;

	        while (bytes_remaining > 0)
	        {
	          if (bytes_remaining > 65535)
	          {
	            chunk_size = 65535;
	          }
	          else
	          {
	            chunk_size = (uint16_t)bytes_remaining;
	          }

	          USB_Transmit_Blocking(p_buffer, chunk_size);
	          p_buffer += chunk_size;
	          bytes_remaining -= chunk_size;
	        }
	        HAL_Delay(1000);
	  }
//	  USB_Transmit_Blocking((uint8_t*)"USB functional\r\n", (uint16_t)16);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  // Feed the dog
	  g_last_feed_time = HAL_GetTick();
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
  hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
  hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
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
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CC_RESET_GPIO_Port, CC_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CC_SHUTTER_GPIO_Port, CC_SHUTTER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, ENABLE_5V_10V_Pin|ENABLE_1V8_3V3_Pin|ENABLE_NEG10V_Pin|LED1_Pin
                          |ENABLE_15V_Pin|LED4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED3_Pin */
  GPIO_InitStruct.Pin = LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED3_GPIO_Port, &GPIO_InitStruct);

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
                           ENABLE_15V_Pin LED4_Pin */
  GPIO_InitStruct.Pin = ENABLE_5V_10V_Pin|ENABLE_1V8_3V3_Pin|ENABLE_NEG10V_Pin|LED1_Pin
                          |ENABLE_15V_Pin|LED4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : BUTTON_Pin */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED2_Pin */
  GPIO_InitStruct.Pin = LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);

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
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  frame_capture_count++;

  if (frame_capture_count >= 1)
  {
//	  HAL_DCMI_Stop(hdcmi);
	  all_frames_ready = 1;
	  frame_capture_count = 0;
  }
}
void I2C_Scanner(void)
{

    HAL_StatusTypeDef result;
    uint8_t i;

    for (i = 1; i < 128; i++)
    {
        /*
         * HAL_I2C_IsDeviceReady checks if a device acknowledges an address.
         * Params:
         * 1. I2C Handle (&hi2c1)
         * 2. Device Address (shifted left by 1)
         * 3. Number of trials (1 is usually enough)
         * 4. Timeout in ms (10ms)
         */
        result = HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(i << 1), 1, 10);

        if (result == HAL_OK)
        {
        	HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
        }
    }
}

static void USB_Transmit_Blocking(uint8_t* Buf, uint16_t Len)
{
//	uint32_t start = HAL_GetTick();

	if (hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED) return;

    USBD_CDC_HandleTypeDef* hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;

    // Wait until previous transfer completes
    while (hcdc->TxState != 0)
    {
        // Optionally add a timeout here to avoid infinite lockup
    }

    // Queue new transfer
    if (CDC_Transmit_HS(Buf, Len) != USBD_OK) return;

    // Wait until this one completes
    while (hcdc->TxState != 0)
    {
        // Optionally add small delay for CPU efficiency
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* Check if the interrupt came from BUTTON */
    if (GPIO_Pin == BUTTON_Pin) Error_Handler();
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
  MPU_InitStruct.Size = MPU_REGION_SIZE_4KB;
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
	HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);

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
