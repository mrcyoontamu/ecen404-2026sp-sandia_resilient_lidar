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
#include <string.h>

#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Compile-time test configuration
#define PAYLOAD_SIZE_BYTES 614400
// Valid sizes to try: 4096, 32768, 61440, 32764, 614400

#define CHUNK_SIZE_BYTES 32768
// Valid chunk sizes to try: 2048, 8192, 32768

#define SEND_PERIOD_MS 2000
#define TX_WAIT_TIMEOUT_MS 5000
#define HOST_TRIGGER_MODE 0
// Set to 1 to send a single frame only after host sends 'G'
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c2;

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceHS;

static uint8_t payload_buffer[PAYLOAD_SIZE_BYTES];

volatile uint32_t frames_attempted = 0;
volatile uint32_t frames_sent_ok = 0;
volatile uint32_t frames_aborted = 0;
volatile uint32_t chunks_sent_ok = 0;
volatile uint32_t chunks_failed = 0;
volatile uint32_t tx_wait_timeout_count = 0;
volatile uint8_t g_usb_rx_trigger_g = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */
static void Fill_Payload(uint32_t sequence, uint8_t *buf, uint32_t len);
static uint8_t USB_Transmit_Blocking(uint8_t *buf, uint16_t len);
static uint8_t USB_Transmit_Buffer_Blocking(const uint8_t *buf, uint32_t len);
static void Send_Test_Frame(uint32_t sequence);
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
  MX_I2C2_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  uint32_t next_send_tick = HAL_GetTick();
  uint32_t sequence = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    uint32_t now = HAL_GetTick();

    if (HOST_TRIGGER_MODE)
    {
      if (CDC_ConsumeTrigger_G())
      {
        Send_Test_Frame(sequence++);
      }
    }
    else
    {
      if ((int32_t)(now - next_send_tick) >= 0)
      {
        Send_Test_Frame(sequence++);
        next_send_tick = now + SEND_PERIOD_MS;
      }
    }

    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    HAL_Delay(1);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

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
static void Fill_Payload(uint32_t sequence, uint8_t *buf, uint32_t len)
{
  uint32_t i;
  for (i = 0; i < len; i++)
  {
    buf[i] = (uint8_t)((sequence + i) & 0xFFu);
  }
}

static uint8_t USB_Transmit_Blocking(uint8_t *buf, uint16_t len)
{
  USBD_CDC_HandleTypeDef *hcdc;
  uint32_t start_tick;

  if (hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED)
  {
    return 0;
  }

  hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
  start_tick = HAL_GetTick();
  while ((hcdc != NULL) && (hcdc->TxState != 0))
  {
    if ((HAL_GetTick() - start_tick) > TX_WAIT_TIMEOUT_MS)
    {
      tx_wait_timeout_count++;
      return 0;
    }
  }

  if (CDC_Transmit_HS(buf, len) != USBD_OK)
  {
    return 0;
  }

  start_tick = HAL_GetTick();
  while ((hcdc != NULL) && (hcdc->TxState != 0))
  {
    if ((HAL_GetTick() - start_tick) > TX_WAIT_TIMEOUT_MS)
    {
      tx_wait_timeout_count++;
      return 0;
    }
  }

  return 1;
}

static uint8_t USB_Transmit_Buffer_Blocking(const uint8_t *buf, uint32_t len)
{
  uint32_t remaining = len;
  const uint8_t *p = buf;

  while (remaining > 0)
  {
    uint16_t chunk = (remaining > CHUNK_SIZE_BYTES) ? (uint16_t)CHUNK_SIZE_BYTES : (uint16_t)remaining;

    if (!USB_Transmit_Blocking((uint8_t*)p, chunk))
    {
      chunks_failed++;
      return 0;
    }

    chunks_sent_ok++;
    p += chunk;
    remaining -= chunk;
  }

  return 1;
}

static void Send_Test_Frame(uint32_t sequence)
{
  uint8_t header[12];

  frames_attempted++;

  header[0] = 0xAA;
  header[1] = 0x55;
  header[2] = 0xAA;
  header[3] = 0x55;
  header[4] = (uint8_t)(sequence & 0xFFu);
  header[5] = (uint8_t)((sequence >> 8) & 0xFFu);
  header[6] = (uint8_t)((sequence >> 16) & 0xFFu);
  header[7] = (uint8_t)((sequence >> 24) & 0xFFu);
  header[8] = (uint8_t)(PAYLOAD_SIZE_BYTES & 0xFFu);
  header[9] = (uint8_t)((PAYLOAD_SIZE_BYTES >> 8) & 0xFFu);
  header[10] = (uint8_t)((PAYLOAD_SIZE_BYTES >> 16) & 0xFFu);
  header[11] = (uint8_t)((PAYLOAD_SIZE_BYTES >> 24) & 0xFFu);

  Fill_Payload(sequence, payload_buffer, PAYLOAD_SIZE_BYTES);

  if (!USB_Transmit_Blocking(header, sizeof(header)))
  {
    frames_aborted++;
    return;
  }

  if (!USB_Transmit_Buffer_Blocking(payload_buffer, PAYLOAD_SIZE_BYTES))
  {
    frames_aborted++;
    return;
  }

  frames_sent_ok++;
}
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
