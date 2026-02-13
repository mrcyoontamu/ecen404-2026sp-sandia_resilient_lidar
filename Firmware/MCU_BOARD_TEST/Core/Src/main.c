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

#include "common.h"
#include "ov7670.h"

#include "core_cm7.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IMG_WIDTH 320
#define IMG_HEIGHT 240
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

uint16_t quad_frame_buffer[IMG_WIDTH*IMG_HEIGHT*NUM_FRAMES];

// One buffer to reference frame angles
uint16_t reference_angle_buffer[SINGLE_FRAME_SIZE];

// One buffer for current frame angles
uint16_t current_angle_buffer[SINGLE_FRAME_SIZE];

// A "flag" to signal when a frame is ready to be sent
volatile uint8_t frame_ready = 0;

// Some timer variables to be used with DWT
volatile uint32_t tDCMI_start, tDCMI_end;
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

void vprint(const char *fmt, va_list argp);
void my_printf(const char *fmt, ...);

void usb_printf(char *msg);
static void USB_Transmit_Blocking(uint8_t* Buf, uint16_t Len);

void DWT_Init(void);
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
  DWT_Init();

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET); //Camera PWDN to GND
  ov7670_init(&hdcmi, &hdma_dcmi, &hi2c2);
  ov7670_config(OV7670_MODE_QVGA_RGB565);
  ov7670_stopCap();

  const uint8_t FRAME_HEADER[4] = {0xAA, 0x55, 0xAA, 0x55};

  uint32_t last_frame_tick = 0;
  uint32_t loop_tick = 0;
  uint32_t frame_timeout_ms = 1000;

  float cpu_freq_mhz = (float)HAL_RCC_GetHCLKFreq() / 1000000.0f;

  // Variables for timing
  uint32_t start_cycles, end_cycles, delta_cycles;
  float time_us;

  HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)frame_buffer, IMG_WIDTH * IMG_HEIGHT / 2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // Blinking light to make sure main loop is still running
	  if (HAL_GetTick() - loop_tick > frame_timeout_ms)
	  {
		  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		  loop_tick = HAL_GetTick();
	  }

	  if(frame_ready == 1)
	  {

//		  my_printf("DCMI frame capture duration: %.3f\r\n", (float)(tDCMI_end - tDCMI_start) / cpu_freq_mhz);

		  frame_ready = 0;

		  // Marked for death, I think this is useless
		  while (HAL_DCMI_GetState(&hdcmi) != HAL_DCMI_STATE_READY)
		  {
			  HAL_Delay(100);// optional: add timeout protection here
		  }



		  start_cycles = DWT->CYCCNT;
		  while(hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED);	// Waiting loop

		  USB_Transmit_Blocking((uint8_t*)FRAME_HEADER, 4);
		  uint8_t* p_buffer = (uint8_t*)frame_buffer;
		  uint32_t bytes_remaining = 320 * 240 * 2; // 153,600
		  uint16_t chunk_size;

		  while (bytes_remaining > 0)
		  {
			  // Calculate the size of the next chunk
			  if (bytes_remaining > 65535) {
				  chunk_size = 65535;
			  } else {
				  chunk_size = (uint16_t)bytes_remaining;
			  }

			  // Send the chunk
			  USB_Transmit_Blocking(p_buffer, chunk_size);

			  // Move the pointer and update the remaining count
			  p_buffer += chunk_size;
			  bytes_remaining -= chunk_size;
		  }
		  end_cycles = DWT->CYCCNT;
//		  my_printf("USB frame transmission: %.3f\r\n", (float)(end_cycles - start_cycles) / cpu_freq_mhz);
		  HAL_DCMI_Stop(&hdcmi);

		  // Start our tDCMI counter
		  tDCMI_start = DWT->CYCCNT;
		  HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)frame_buffer, IMG_WIDTH * IMG_HEIGHT / 2);
		  last_frame_tick = HAL_GetTick();
	  }
	  if (HAL_GetTick() - last_frame_tick > frame_timeout_ms)
	  {
		  HAL_DCMI_Stop(&hdcmi);
		  HAL_DCMI_DeInit(&hdcmi);
		  MX_DCMI_Init(); // call your CubeMX-generated init function again

		  HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)frame_buffer, IMG_WIDTH * IMG_HEIGHT / 2);
		  last_frame_tick = HAL_GetTick();
	  }
// SOME OLD CODE TO TEST I2C AND USB
//	  I2C_Scanner();
//	  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
//	  static uint32_t counter = 0;
//
//	  // 1. Format the string directly into the Safe Buffer
//	  // (This works safely because UserTxBufferHS is in the MPU protected area)
//	  int len = sprintf((char *)UserTxBufferHS, "STM32 Alive! Count: %lu\r\n", counter++);
//
//	  // 2. Send the data
//	  // We check if the USB is free (USBD_OK) before sending
//	  if (CDC_Transmit_HS(UserTxBufferHS, len) == USBD_OK)
//	  {
//		  // Optional: Toggle an LED here to visualize successful transmission
//		  // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//	  }
//	  else
//	  {
//		  // If USB was busy, we just skip this frame and try again next time
//	  }
//
//	  // 3. Wait a bit so we don't flood the terminal
//	  HAL_Delay(1000);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
  hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
  hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
  hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
  hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
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
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LED_Pin|GPIO_PIN_2|CAMERA_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_Pin PD2 CAMERA_RESET_Pin */
  GPIO_InitStruct.Pin = LED_Pin|GPIO_PIN_2|CAMERA_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/************************************************* USER DEFINED FUNCTIONS ********************************************/
/************************************************* USER DEFINED FUNCTIONS ********************************************/
/************************************************* USER DEFINED FUNCTIONS ********************************************/
//void I2C_Scanner(void)
//{
//    char msg[64];
//    HAL_UART_Transmit(&huart6, (uint8_t*)"Scanning I2C bus...\r\n", 21, 100);
//
//    HAL_StatusTypeDef result;
//    uint8_t i;
//    int count = 0;
//
//    for (i = 1; i < 128; i++)
//    {
//        /*
//         * HAL_I2C_IsDeviceReady checks if a device acknowledges an address.
//         * Params:
//         * 1. I2C Handle (&hi2c1)
//         * 2. Device Address (shifted left by 1)
//         * 3. Number of trials (1 is usually enough)
//         * 4. Timeout in ms (10ms)
//         */
//        result = HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(i << 1), 1, 10);
//
//        if (result == HAL_OK)
//        {
//            sprintf(msg, "I2C device found at address: 0x%02X\r\n", i);
//            HAL_UART_Transmit(&huart6, (uint8_t*)msg, strlen(msg), 100);
//            count++;
//        }
//    }
//
//    if (count == 0)
//    {
//        HAL_UART_Transmit(&huart6, (uint8_t*)"No I2C devices found.\r\n", 25, 100);
//    }
//    else
//    {
//        HAL_UART_Transmit(&huart6, (uint8_t*)"Scan Complete.\r\n", 16, 100);
//    }
//}
//void vprint(const char *fmt, va_list argp) {
//	char string[200];
//	if (0 < vsprintf(string, fmt, argp)) // build string
//	{
//		HAL_UART_Transmit(&huart3, (uint8_t*) string, strlen(string), 0xffffff); // send message via UART
//	}
//}
//
//void my_printf(const char *fmt, ...) // custom printf() function
//{
//	va_list argp;
//	va_start(argp, fmt);
//	vprint(fmt, argp);
//	va_end(argp);
//}
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  // This function is called by the HAL when the DMA has
  // successfully captured a complete frame.

  // 1. Set our flag so the main loop knows it can send the data
  frame_ready = 1;
  tDCMI_end = DWT->CYCCNT;

//  // 2. Stop the DCMI from capturing more frames
//  //    We will restart it manually after we're done sending.
}

void usb_printf(char *msg)
{
	if (hUsbDeviceHS.dev_state == USBD_STATE_CONFIGURED)
	{
	  // 3. Send the data
	  CDC_Transmit_HS((uint8_t*)msg, strlen(msg));
	}
}

static void USB_Transmit_Blocking(uint8_t* Buf, uint16_t Len)
{
    USBD_CDC_HandleTypeDef* hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;

    // Wait until previous transfer completes
    while (hcdc->TxState != 0)
    {
        // Optionally add a timeout here to avoid infinite lockup
    }

    // Queue new transfer
    CDC_Transmit_HS(Buf, Len);

    // Wait until this one completes
    while (hcdc->TxState != 0)
    {
        // Optionally add small delay for CPU efficiency
    }
}

void DWT_Init(void)
{
  if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
  {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  }
  if (!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk))
  {
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  }
  DWT->CYCCNT = 0;
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
  __disable_irq();
  while (1)
  {
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
