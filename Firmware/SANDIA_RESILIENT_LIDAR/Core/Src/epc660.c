/*
 * epc660.c
 *
 *  Created on: Jan 22, 2026
 *      Author: unrea
 */

// INCLUDES
#include "epc660.h"
#include "epc660_reg.h"
#include "epc660_platform.h"

// DEFINES

// VARIABLES

// PRIVATE HELPER FUNCTION

// --------------------FUNCTION DEFINITIONS--------------------
epc_status_t epc660_power_up(void)
{
	// the epc660 RESET pin is active low
	epc_reset_pin_set(0);
	epc_shutter_pin_set(0);
	epc_delay_ms(5);

	// Digital power
	epc_power_1v8_3v3_set(1);
	epc_delay_ms(10);

	// Analog power
	epc_power_5v_10v_set(1);
	epc_delay_ms(10);

	// Negative bias
	epc_power_neg10v_set(1);
	epc_delay_ms(10);

	// Release reset
	epc_reset_pin_set(1);
	epc_delay_ms(5);

	// Check if epc is alive
	uint8_t chip_id = 0;
	if (epc_i2c_read(EPC_REG_IC_TYPE, &chip_id, EPC_DIRECT) != EPC_OK)
	{
		return EPC_ERR;
	}

	// Check READY state (should take < 1ms)
	uint8_t status = 0;

	// bit 0 == 1 of register 0x7E indicates EEPROM successfully copied to CFG
	epc_i2c_read(EPC_REG_CFG_READY, &status, EPC_DIRECT);
	if ((status & 0x01) == 0) {
		return EPC_ERR;
	}

	return EPC_OK;	// Everything was successful
}

void ecp660_power_down(void)
{
	// the epc660 RESET pin is active low
	epc_reset_pin_set(0);
	epc_shutter_pin_set(0);
	epc_delay_ms(5);

	// Negative bias
	epc_power_neg10v_set(0);
	epc_delay_ms(10);

	// Analog power
	epc_power_5v_10v_set(0);
	epc_delay_ms(10);

	// Digital power
	epc_power_1v8_3v3_set(0);
	epc_delay_ms(10);
}

void epc_emergency_power_down(void)
{
	// the epc660 RESET pin is active low
	epc_reset_pin_set(0);
	epc_shutter_pin_set(0);
	epc_emergency_delay(2400000);	// Very rough 10ms delay

	// Negative bias
	epc_power_neg10v_set(0);
	epc_emergency_delay(2400000);

	// Analog power
	epc_power_5v_10v_set(0);
	epc_emergency_delay(2400000);

	// Digital power
	epc_power_1v8_3v3_set(0);
	epc_emergency_delay(2400000);
}

