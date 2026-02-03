/*
 * epc660_platform.c
 *
 *  Created on: Jan 22, 2026
 *      Author: unrea
 */

#include "epc660_platform.h"
#include "epc660_reg.h"

#include "main.h"
#include "stm32h7xx_hal.h"


// PRIVATE DEFINES
#define EPC_TIMEOUT 100

// VARIABLE DECLARATION
extern I2C_HandleTypeDef hi2c2;

// PRIVATE HELPER FUNCTIONS
static epc_status_t _map_hal_status(HAL_StatusTypeDef status)
{
	switch(status)
	{
		case HAL_OK:      return EPC_OK;
		case HAL_BUSY:    return EPC_ERR_BUSY;
		case HAL_TIMEOUT: return EPC_ERR_TIMEOUT;
		default:          return EPC_ERR;
	}
}

// I2C helper functions
static epc_status_t _write_direct(uint8_t reg_addr, uint8_t value)
{
	HAL_StatusTypeDef status;

	status = HAL_I2C_Mem_Write(&hi2c2, EPC_I2C_ADDR,
								reg_addr, I2C_MEMADD_SIZE_8BIT,
								&value, 1, EPC_TIMEOUT);

	return _map_hal_status(status);
}
static epc_status_t _read_direct(uint8_t reg_addr, uint8_t *value)
{
	HAL_StatusTypeDef status;

	status = HAL_I2C_Mem_Read(&hi2c2, EPC_I2C_ADDR,
								reg_addr, I2C_MEMADD_SIZE_8BIT,
								value, 1, EPC_TIMEOUT);

	return _map_hal_status(status);
}
static epc_status_t _write_indirect(uint8_t target_addr, uint8_t value)
{
	epc_status_t status;

    // 1. Write the target address to the POINTER register (e.g., 0x11)
	status = _write_direct(0x11, target_addr);
    if (status != EPC_OK) return status;

    // 2. Write the value to the DATA register (e.g., 0x12)
    status = _write_direct(0x12, value);
    if (status != EPC_OK) return status;

    // Datasheet says EEPROM indirect writes take 20ms
    epc_delay_ms(20);

    return EPC_OK;
}
static epc_status_t _read_indirect(uint8_t target_addr, uint8_t *value)
{
	epc_status_t status;

    // 1. Write the target address to the POINTER register
	status = _write_direct(0x11, target_addr);
    if (status != EPC_OK) return status;

    // 2. Read the value from the DATA register
    status = _read_direct(0x12, value);
    if (status != EPC_OK) return status;

    return EPC_OK;
}


// ----------FUNCTION DEFINITIONS----------
// RESET FUNCTIONS
void epc_reset_pin_set(int state)
{
	if (state == 1){
		HAL_GPIO_WritePin(CC_RESET_GPIO_Port, CC_RESET_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(CC_RESET_GPIO_Port, CC_RESET_Pin, GPIO_PIN_RESET);
	}
}

void epc_shutter_pin_set(int state)
{
	if (state == 1){
		HAL_GPIO_WritePin(CC_SHUTTER_GPIO_Port, CC_SHUTTER_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(CC_SHUTTER_GPIO_Port, CC_SHUTTER_Pin, GPIO_PIN_RESET);
	}
}


// VOLTAGE FUNCTIONS
void epc_power_1v8_3v3_set(int state)
{
	if (state == 1){
		HAL_GPIO_WritePin(ENABLE_1V8_3V3_GPIO_Port, ENABLE_1V8_3V3_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(ENABLE_1V8_3V3_GPIO_Port, ENABLE_1V8_3V3_Pin, GPIO_PIN_RESET);
	}
}

void epc_power_5v_10v_set(int state)
{
	if (state == 1){
		HAL_GPIO_WritePin(ENABLE_5V_10V_GPIO_Port, ENABLE_5V_10V_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(ENABLE_5V_10V_GPIO_Port, ENABLE_5V_10V_Pin, GPIO_PIN_RESET);
	}
}

void epc_power_neg10v_set(int state)
{
	if (state == 1){
		HAL_GPIO_WritePin(ENABLE_NEG10V_GPIO_Port, ENABLE_NEG10V_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(ENABLE_NEG10V_GPIO_Port, ENABLE_NEG10V_Pin, GPIO_PIN_RESET);
	}
}
void epc_power_15v_set(int state)
{
	if (state == 1){
		HAL_GPIO_WritePin(ENABLE_15V_GPIO_Port, ENABLE_15V_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(ENABLE_15V_GPIO_Port, ENABLE_15V_Pin, GPIO_PIN_RESET);
	}
}


// TIME FUNCTIONS
void epc_delay_ms(uint32_t ms)
{
	HAL_Delay(ms);
}

void epc_emergency_delay(uint32_t loop_count)
{
    for (volatile uint32_t i = 0; i < loop_count; i++) {
        __asm("nop"); // Assembly "No Operation" to prevent optimization
    }
}


// I2C FUNCTIONS
epc_status_t epc_i2c_write(uint8_t reg_addr, uint8_t value, epc_i2c_access_t type)
{
	if (type == EPC_DIRECT)
	{
		return _write_direct(reg_addr, value);
	}
	else
	{
		return _write_indirect(reg_addr, value);
	}
}

epc_status_t epc_i2c_read(uint8_t reg_addr, uint8_t *value, epc_i2c_access_t type)
{
	if (type == EPC_DIRECT)
	{
		return _read_direct(reg_addr, value);
	}
	else
	{
		return _read_indirect(reg_addr, value);
	}
}

epc_status_t epc_i2c_write_multi(uint8_t reg_addr, const uint8_t *data, uint16_t len)
{
	HAL_StatusTypeDef status;

	status = HAL_I2C_Mem_Write(&hi2c2,
			EPC_I2C_ADDR,
			reg_addr,
			I2C_MEMADD_SIZE_8BIT,
			(uint8_t*)data,
			len,
			EPC_TIMEOUT);

	return _map_hal_status(status);
}

epc_status_t epc_i2c_read_multi(uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status;

    // HAL_I2C_Mem_Read automatically handles the register address
    // and then clocks out 'len' number of bytes.
    status = HAL_I2C_Mem_Read(&hi2c2,
                                EPC_I2C_ADDR,
                                reg_addr,
                                I2C_MEMADD_SIZE_8BIT,
                                data,
                                len,
                                EPC_TIMEOUT);

    return _map_hal_status(status);
}
