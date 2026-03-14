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

//	// Ensures the chip is in ambient only mode (I should write this register in a separate function)
//	if (epc_i2c_write(0x3C, 0x26, EPC_DIRECT) != EPC_OK) {
//		return EPC_ERR;
//	}
	return EPC_OK;
}

epc_status_t epc660_soft_reset(void)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Performs a software reset of the EPC660 using the I2C general call
	 * command. This resets all registers to their default values and
	 * triggers a reload of EEPROM contents into configuration registers.
	 * After calling this, you must wait for EEPROM->CFG copy and re-init.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Send I2C general call software reset command:
	 *    - I2C address: 0x00 (general call)
	 *    - Data byte: 0x06 (software reset command)
	 *    - This is a special I2C transaction, may need raw I2C API
	 *
	 * 2. Wait for reset to complete (~1ms minimum)
	 *    - epc_delay_ms(2)
	 *
	 * 3. Wait for EEPROM->CFG copy (register 0x7E bit 0 == 1)
	 *    - Poll with timeout
	 *
	 * 4. Return EPC_OK on success
	 *
	 * NOTE: After soft reset, epc660_init() should be called again
	 *       to reload sequencer and apply configuration.
	 */

	return EPC_OK;
}

// Seems like a useless abstraction tbh
//epc_status_t epc660_load_sequencer(void)
//{
//	/*
//	 * HIGH-LEVEL DESCRIPTION:
//	 * -----------------------
//	 * Wrapper function that loads the pixel sequencer code into the
//	 * EPC660's RAM. The sequencer code controls the pixel timing and
//	 * is required for proper operation. This delegates to the
//	 * epc660_sequencer_load() function in epc660_sequencer.c.
//	 *
//	 * PSEUDOCODE:
//	 * -----------
//	 * 1. Include epc660_sequencer.h at top of file
//	 *
//	 * 2. Call epc660_sequencer_load()
//	 *    - This function writes the sequencer table to RAM via I2C
//	 *
//	 * 3. Return the status from epc660_sequencer_load()
//	 */
//
//	return EPC_OK;
//}

epc_status_t epc660_trigger_single_measurement(void)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Triggers a single measurement cycle using the software shutter
	 * control register. The chip will acquire 1, 2, or 4 DCS frames
	 * depending on the current mode setting (register 0x92).
	 * Data will be output via TCMI after integration and ADC conversion.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Write register 0xA4 with bit 0 = 1
	 *    - epc_i2c_write(0xA4, 0x01, EPC_DIRECT)
	 *    - Bit 0 is auto-cleared after triggering
	 *
	 * 2. Return status from the I2C write
	 *
	 * NOTE: This function returns immediately. The actual measurement
	 *       takes time depending on integration time setting.
	 *       Monitor VSYNC to know when data is ready.
	 *       SW shutter has ~26us more latency than HW shutter.
	 */

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

epc_status_t epc660_get_status(uint8_t *status_out)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Reads the configuration ready status register (0x7E).
	 * Bit 0 indicates whether EEPROM->CFG copy is complete.
	 * Useful for checking if the chip is ready after reset.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Validate input pointer
	 *    - If status_out == NULL, return EPC_ERR_PARAM
	 *
	 * 2. Read register 0x7E
	 *    - epc_i2c_read(0x7E, status_out, EPC_DIRECT)
	 *
	 * 3. Return status from I2C read
	 *
	 * STATUS BITS:
	 *   Bit 0: EEPROM->CFG copy complete (1 = ready)
	 */

	return EPC_OK;
}

epc_status_t epc660_get_id(uint8_t *chip_id_out, uint8_t *ic_version_out)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Reads the chip identification registers:
	 *   - IC Type (0x01): Identifies the chip model
	 *   - Part Version (0xFB): Identifies the chip revision
	 * Useful for verifying correct chip and firmware compatibility.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Validate input pointers
	 *    - Either can be NULL if caller doesn't need that value
	 *
	 * 2. If chip_id_out != NULL:
	 *    - Read register 0x01
	 *    - epc_i2c_read(0x01, chip_id_out, EPC_DIRECT)
	 *    - Expected value: 0x06 for EPC660
	 *
	 * 3. If ic_version_out != NULL:
	 *    - Read register 0xFB (via EEPROM indirect read)
	 *    - epc_i2c_read(0xFB, ic_version_out, EPC_INDIRECT)
	 *
	 * 4. Return EPC_OK if all reads succeeded
	 */

	return EPC_OK;
}

epc_status_t epc660_get_unique_id(uint16_t *wafer_id_out, uint16_t *chip_id_out)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Reads the unique identification data from EEPROM:
	 *   - Wafer ID (0xFC-0xFD): 16-bit wafer identifier
	 *   - Chip ID (0xFE-0xFF): 16-bit unique chip identifier
	 * Together these form a globally unique ID for each chip.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Validate input pointers
	 *    - Either can be NULL if caller doesn't need that value
	 *
	 * 2. If wafer_id_out != NULL:
	 *    - Read EEPROM 0xFC (high byte) via indirect access
	 *    - Read EEPROM 0xFD (low byte) via indirect access
	 *    - Combine: *wafer_id_out = (high << 8) | low
	 *
	 * 3. If chip_id_out != NULL:
	 *    - Read EEPROM 0xFE (high byte) via indirect access
	 *    - Read EEPROM 0xFF (low byte) via indirect access
	 *    - Combine: *chip_id_out = (high << 8) | low
	 *
	 * 4. Return EPC_OK if all reads succeeded
	 *
	 * NOTE: Uses indirect EEPROM access (pointer reg 0x11, data reg 0x12)
	 */

	return EPC_OK;
}

epc_status_t epc660_capture_frame(uint16_t *buffer, uint32_t buffer_len, uint32_t *bytes_written)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Captures a complete DCS or grayscale frame from the TCMI parallel
	 * interface. This function is a PLACEHOLDER - actual implementation
	 * depends heavily on your MCU's peripheral setup (DCMI, GPIO+DMA, etc.).
	 *
	 * The EPC660 outputs data via:
	 *   - DATA[11:0]: 12-bit pixel data
	 *   - DCLK: Pixel clock (up to 48MHz)
	 *   - VSYNC: Frame valid signal
	 *   - HSYNC: Line valid signal
	 *   - XSYNC_SAT: Saturation flag
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Validate input parameters
	 *    - If buffer == NULL, return EPC_ERR_PARAM
	 *    - If buffer_len < required size, return EPC_ERR_PARAM
	 *    - Required size = width * height * 2 bytes (for 12-bit packed in 16)
	 *
	 * 2. Configure DMA/DCMI for capture
	 *    - Set destination buffer address
	 *    - Set transfer size based on current ROI/mode settings
	 *    - Enable VSYNC/HSYNC synchronization
	 *
	 * 3. Trigger measurement
	 *    - epc660_trigger_single_measurement() or epc660_trigger_hw_shutter()
	 *
	 * 4. Wait for capture complete
	 *    - Poll DMA complete flag or use interrupt/semaphore
	 *    - Timeout if frame not received in expected time
	 *
	 * 5. Set *bytes_written = actual bytes captured
	 *
	 * 6. Return EPC_OK on success
	 *
	 * IMPLEMENTATION OPTIONS:
	 *   A. STM32 DCMI peripheral (if pins match)
	 *   B. GPIO + DMA with external trigger on DCLK
	 *   C. FSMC/FMC in parallel interface mode
	 *   D. Software bit-bang (very slow, not recommended)
	 */

	(void)buffer;       // Suppress unused parameter warning
	(void)buffer_len;
//	if (bytes_written != NULL) {
//		*bytes_written = 0;
//	}
	return EPC_ERR;     // Not implemented - return error
}

epc_status_t epc660_read_reg(uint8_t reg_addr, uint8_t *value_out)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Reads a single register from the EPC660 via I2C.
	 * Direct access covers registers 0x00-0x7F (control) and 0x80-0xEF (RAM).
	 * For EEPROM (0xF0-0xFF), use epc660_read_eeprom() instead.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Validate input pointer
	 *    - If value_out == NULL, return EPC_ERR_PARAM
	 *
	 * 2. Perform I2C read
	 *    - epc_i2c_read(reg_addr, value_out, EPC_DIRECT)
	 *
	 * 3. Return status from I2C read
	 */

	return EPC_OK;
}

epc_status_t epc660_write_reg(uint8_t reg_addr, uint8_t value)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Writes a single register on the EPC660 via I2C.
	 * Direct access covers registers 0x00-0x7F (control) and 0x80-0xEF (RAM).
	 * For EEPROM (0xF0-0xFF), use epc660_write_eeprom() instead.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Perform I2C write
	 *    - epc_i2c_write(reg_addr, value, EPC_DIRECT)
	 *
	 * 2. Return status from I2C write
	 *
	 * WARNING: Some registers should not be modified during measurement.
	 *          Refer to datasheet for timing constraints.
	 */

	return EPC_OK;
}

epc_status_t epc660_read_eeprom(uint8_t eeprom_addr, uint8_t *value_out)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Reads a byte from the EPC660's internal EEPROM using indirect access.
	 * EEPROM contains factory calibration data and unique chip ID.
	 * Address range: 0xF0-0xFF (128 bytes).
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Validate input pointer
	 *    - If value_out == NULL, return EPC_ERR_PARAM
	 *
	 * 2. Write target address to pointer register
	 *    - epc_i2c_write(0x11, eeprom_addr, EPC_DIRECT)
	 *
	 * 3. Read data from data register
	 *    - epc_i2c_read(0x12, value_out, EPC_DIRECT)
	 *
	 * 4. Return EPC_OK on success
	 *
	 * NOTE: Can also use epc_i2c_read() with EPC_INDIRECT flag
	 *       which encapsulates steps 2-3.
	 */

	return EPC_OK;
}

epc_status_t epc660_write_eeprom(uint8_t eeprom_addr, uint8_t value)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Writes a byte to the EPC660's internal EEPROM using indirect access.
	 * EEPROM writes are SLOW (~25ms per byte) and have limited endurance.
	 * Use sparingly - typically only for storing calibration data.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Write target address to pointer register
	 *    - epc_i2c_write(0x11, eeprom_addr, EPC_DIRECT)
	 *
	 * 2. Write data to data register
	 *    - epc_i2c_write(0x12, value, EPC_DIRECT)
	 *
	 * 3. Wait for EEPROM write to complete
	 *    - epc_delay_ms(25)  // Datasheet specifies 25ms per byte
	 *
	 * 4. Return EPC_OK on success
	 *
	 * WARNING: EEPROM has limited write endurance (~100k cycles).
	 *          Do not write in a loop or frequently during operation.
	 *
	 * NOTE: Can also use epc_i2c_write() with EPC_INDIRECT flag
	 *       which encapsulates steps 1-3.
	 */

	return EPC_OK;
}

epc_status_t epc660_load_calibration(void)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Loads calibration parameters from EEPROM into active configuration
	 * registers. This happens automatically after reset, but can be
	 * called manually if calibration data was modified.
	 *
	 * The EEPROM contains factory-programmed calibration values that
	 * are copied to configuration registers during startup.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Option A: Trigger a soft reset to reload everything
	 *    - epc660_soft_reset()
	 *    - Wait for EEPROM->CFG copy complete
	 *    - Reload sequencer code
	 *
	 * 2. Option B: Manually copy specific calibration registers
	 *    - Read EEPROM addresses containing calibration data
	 *    - Write to corresponding configuration registers
	 *    - Calibration locations are chip-version specific
	 *
	 * 3. Return EPC_OK on success
	 *
	 * NOTE: The specific calibration registers depend on the chip
	 *       version and application. Refer to ESPROS AN10
	 *       "Calibration and Compensation" for details.
	 */

	return EPC_OK;
}


