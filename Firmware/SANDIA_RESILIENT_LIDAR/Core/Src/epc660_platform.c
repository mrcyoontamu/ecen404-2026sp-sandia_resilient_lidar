/*
 * epc660_platform.c
 *
 *  Created on: Jan 22, 2026
 *      Author: unrea
 */

#include "epc660_platform.h"
#include "main.h"
#include "stm32h7xx_hal.h"

// RESET FUNCTIONS
void epc_reset_pin_set_low(void)
{
	HAL_GPIO_WritePin(CC_RESET_GPIO_Port, CC_RESET_Pin, GPIO_PIN_RESET);
}
void epc_reset_pin_set_high(void)
{
	HAL_GPIO_WritePin(CC_RESET_GPIO_Port, CC_RESET_Pin, GPIO_PIN_SET);
}

// VOLTAGE FUNCTIONS
void epc_power_1v8_3v3_on(void)
{
	HAL_GPIO_WritePin(ENABLE_1V8_3V3_GPIO_Port, ENABLE_1V8_3V3_Pin, GPIO_PIN_SET);
}

void epc_power_5v_10v_on(void)
{
	HAL_GPIO_WritePin(ENABLE_5V_10V_GPIO_Port, ENABLE_5V_10V_Pin, GPIO_PIN_SET);
}
void epc_power_neg10v_on(void)
{
	HAL_GPIO_WritePin(ENABLE_NEG10V_GPIO_Port, ENABLE_NEG10V_Pin, GPIO_PIN_SET);
}
void epc_power_15v_on(void)
{
	HAL_GPIO_WritePin(ENABLE_15V_GPIO_Port, ENABLE_15V_Pin, GPIO_PIN_SET);
}

// TIME FUNCTIONS
void epc_delay_ms(uint32_t ms)
{
	HAL_Delay(ms);
}

// I2C FUNCTIONS
int epc_i2c_write(uint8_t reg_addr, uint8_t value)
{

}
int epc_i2c_read(uint8_t reg_addr, uint8_t *value)
{

}
