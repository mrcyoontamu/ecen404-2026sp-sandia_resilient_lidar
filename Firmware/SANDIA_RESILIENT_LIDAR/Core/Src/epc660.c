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

// FUNCTION DEFINITIONS
epc_status_t epc660_power_up(void)
{
	// the epc660 RESET pin is active low
	epc_reset_pin_set(0);
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

//	// 1. Safety: Hold RESET Low immediately
//	EPC_RESET_LOW();
//
//	// 2. Power Sequence (Datasheet Section 5.6)
//	// If your board has power switches, turn them on here in order:
//	// 1.8V -> 3.3V -> Analog 10V/5V -> Negative Bias
//	EPC_PWR_1V8_ON();
//	EPC_DELAY_MS(5);
//	EPC_PWR_10V_ON();
//	EPC_DELAY_MS(10); // Wait for caps to charge
//
//	// 3. Release RESET to start the chip
//	EPC_RESET_HIGH();
//
//	// 4. Wait for Boot (Datasheet 11.1.1)
//	// The chip needs time to copy data from EEPROM to RAM.
//	// Datasheet says ~340us. We wait 5ms to be safe.
//	EPC_DELAY_MS(5);
//
//	// 5. Verification (Sanity Check)
//	// Read the "Mounting Type Status" register (0x7E)
//	// Bit 0 must be 1 ("Startup finished")
//	uint8_t status = read_reg(EPC_REG_MT_STATUS);
//
//	if ( (status & 0x01) == 0 ) {
//		return EPC_ERR_NOT_READY; // Chip stuck!
//	}
//
//	// Optional: Check Chip ID
//	uint8_t chip_id = read_reg(EPC_REG_IC_TYPE);
//	if (chip_id == 0x00 || chip_id == 0xFF) {
//		return EPC_ERR_I2C; // Bus error
//	}
//
//	return EPC_OK;
}
