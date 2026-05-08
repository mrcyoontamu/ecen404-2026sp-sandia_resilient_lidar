/*
 * epc660.c
 *
 *  Created on: Jan 22, 2026
 *      Author: unrea
 */

// INCLUDES
#include "epc660.h"
#include "epc660_reg.h"
#include "epc660_sequencer.h"
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
	epc_delay_ms(50);

	// Analog power
	epc_power_5v_10v_set(1);
	epc_delay_ms(50);

	// Negative bias
	epc_power_neg10v_set(1);
	epc_delay_ms(50);

	// LED power
	epc_power_15v_set(1);
	epc_delay_ms(50);

	// Release reset
	epc_reset_pin_set(1);
	epc_delay_ms(20);

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

void epc660_power_down(void)
{
	// the epc660 RESET pin is active low
	epc_reset_pin_set(0);
	epc_shutter_pin_set(0);
	epc_delay_ms(5);

	// LED voltage
	epc_power_15v_set(0);
	epc_delay_ms(10);

	// Negative bias
	epc_power_neg10v_set(0);
	epc_delay_ms(10);

	// Analog power
	epc_power_5v_10v_set(0);
	epc_delay_ms(1500);

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

	// LED voltage
	epc_power_15v_set(0);
	epc_emergency_delay(2400000);

	// Negative bias
	epc_power_neg10v_set(0);
	epc_emergency_delay(2400000);

	// Analog power
	epc_power_5v_10v_set(0);
	epc_emergency_delay(240000000);

	// Digital power
	epc_power_1v8_3v3_set(0);
	epc_emergency_delay(2400000);
}

epc_status_t epc660_init(void)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Initializes the EPC660 after power-up sequence is complete.
	 * This function should be called after epc660_power_up() returns successfully.
	 * It verifies the chip is responding, loads the pixel sequencer code,
	 * and applies the default configuration settings per the datasheet.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read chip ID from register 0x01
	 *    - If chip_id != expected value (0x06 for epc660), return EPC_ERR_ID
	 *
	 * 2. Verify EEPROM->CFG copy is complete
	 *    - Read register 0x7E
	 *    - If bit 0 != 1, wait and retry (timeout after ~5ms)
	 *    - If still not ready, return EPC_ERR
	 *
	 * 3. Load pixel sequencer code
	 *    - Call epc660_sequencer_load() from epc660_sequencer.h
	 *    - If failed, return error
	 *
	 * 4. Apply default configuration (datasheet Table 22):
	 *    - Write 0x90 bit 3 = 1 (LED preheat enable)
	 *    - Write 0xAB = 0x01 (LED preheat timing)
	 *    - Write 0xAE = 0x04 (enable manual DLL control)
	 *    - Write 0x3A = 0x30 (differential readout with ABS)
	 *
	 * 5. Return EPC_OK on success
	 */
	epc_status_t status;

	uint8_t chip_id = 0;
	if ((status = epc_i2c_read(0xFA, &chip_id, EPC_INDIRECT)) != EPC_OK) {
		return status;
	}
	if (chip_id != 0x02) {
		return EPC_ERR_ID;
	}

	uint8_t ready_status = 0;
	for (uint32_t i = 0; i < 5; ++i) {
		if ((status = epc_i2c_read(EPC_REG_CFG_READY, &ready_status, EPC_DIRECT)) != EPC_OK) {
			return status;
		}
		if ((ready_status & 0x01) != 0) {
			break;
		}
		epc_delay_ms(1);
	}
	if ((ready_status & 0x01) == 0) {
		return EPC_ERR;
	}

	// Load the pixel sequencer code after each boot-up
	// TODO: I still should implement a read back function to make sure it wrote correctly
	if ((status = epc660_sequencer_load()) != EPC_OK) {
		return status;
	}
	if ((status = epc660_sequencer_readback()) != EPC_OK) return status;

	// Enable LED pre-heat (3rd bit of reg 0x90 to 1 then 0xAB to 0x01)
	uint8_t reg_val = 0;
	if ((status = epc_i2c_read(0x90, &reg_val, EPC_DIRECT)) != EPC_OK) {
		return status;
	}
	reg_val |= (1u << 3);
	if ((status = epc_i2c_write(0x90, reg_val, EPC_DIRECT)) != EPC_OK) {
		return status;
	}
	if (epc_i2c_write(0xAB, 0x01, EPC_DIRECT) != EPC_OK) {
		return EPC_ERR;
	}

	// Enable Manual DLL Control (IMPLEMENT LATER
	// DLL is to insert a phase shift between the modulation of the light source and the
	// demodulation of the pixels to optimize performance
//	if (epc_i2c_write(0xAE, 0x04, EPC_DIRECT) != EPC_OK) {
//		return EPC_ERR;
//	}


	// GRAYSCALE CONFIGURATIONS
	// Set grayscale mode for ABS (This setting can persist even during 4 DCS)
	if (epc_i2c_write(0x3A, 0x30, EPC_DIRECT) != EPC_OK) {
		return EPC_ERR;
	}

	return EPC_OK;
}


void epc660_trigger_hw_shutter(void)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Triggers a single measurement cycle using the hardware SHUTTER pin.
	 * This is faster than SW shutter by ~26us. The SHUTTER pin must be
	 * pulsed high for at least 250ns to trigger.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Set SHUTTER pin HIGH
	 *    - epc_shutter_pin_set(1)
	 *
	 * 2. Wait minimum 250ns (1us is safe with HAL timing)
	 *    - Use a short delay or NOP loop
	 *    - At 480MHz core clock, ~120 cycles = 250ns
	 *
	 * 3. Set SHUTTER pin LOW
	 *    - epc_shutter_pin_set(0)
	 *
	 * 4. Return EPC_OK
	 *
	 * NOTE: For continuous mode, keep SHUTTER high during the last
	 *       row pair readout of each frame. See datasheet section 6.2.2.
	 */
	epc_shutter_pin_set(1);
	epc_delay_us(1);
	epc_shutter_pin_set(0);
}


