/*
 * epc660_config.c
 *
 *  Created on: Jan 22, 2026
 *      Author: unrea
 */

// INCLUDES
#include "epc660_config.h"
#include "epc660_reg.h"
#include "epc660_platform.h"

// DEFINES

// PRIVATE VARIABLES
static uint8_t _saved_trim_d3 = 0;  // Saved trim register for temp sensor
static uint8_t _saved_trim_d5 = 0;
static uint8_t _saved_trim_da = 0;
static uint8_t _saved_trim_dc = 0;
static int16_t _temp_cal_offset[4] = {0}; // Calibration offsets for 4 sensors

// PRIVATE HELPER FUNCTIONS

/* ===== AI-GENERATED BLOCK START ===== */
/*
 * -----------------------------------------------------------------------------
 * STUB IMPLEMENTATIONS FOR EPC660 CONFIGURATION FUNCTIONS
 *
 * Each function below contains:
 *   1. A high-level description of what the function should do
 *   2. A pseudocode section outlining the implementation steps
 *   3. A simple return statement for compilation
 *
 * Replace the return statements with actual implementation code.
 * -----------------------------------------------------------------------------
 */

/*
 * =============================================================================
 * MODULATION & TIMING CONFIGURATION
 * =============================================================================
 */

epc_status_t epc_set_modulation_divider(uint8_t divider)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Sets the LED modulation frequency divider in register 0x85.
	 * The internal 96MHz PLL clock is divided by (divider+1) to produce
	 * the modulation frequency. This directly affects the unambiguity range:
	 *   divider=0 -> 96/(0+1)=96MHz mod -> 24MHz LED -> 6.25m range
	 *   divider=1 -> 96/(1+1)=48MHz mod -> 12MHz LED -> 12.5m range (default)
	 *   divider=3 -> 96/(3+1)=24MHz mod -> 6MHz LED  -> 25m range
	 *   divider=7 -> 96/(7+1)=12MHz mod -> 3MHz LED  -> 50m range
	 *   divider=15 -> 96/(15+1)=6MHz mod -> 1.5MHz LED -> 100m range
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Validate divider is within acceptable range
	 *    - Valid values: 0, 1, 3, 7, 15 (powers of 2 minus 1)
	 *    - Other values technically work but give non-standard frequencies
	 *
	 * 2. Write divider value to register 0x85
	 *    - epc_i2c_write(0x85, divider, EPC_DIRECT)
	 *
	 * 3. Return status from I2C write
	 *
	 * NOTE: Changing modulation frequency affects integration time calculation.
	 *       After changing this, recalculate integration time if needed.
	 */

	epc_status_t status;

	/* Validate allowed divider values: 0, 1, 3, 7, 15 */
	if (!(divider == 0 || divider == 1 || divider == 3 ||
	      divider == 7 || divider == 15) || divider == 31) {
		return EPC_ERR_PARAM;
	}
	if ((status = epc_i2c_write(EPC_REG_MOD_DIV, divider, EPC_DIRECT)) != EPC_OK) {
		return status;
	}
	return EPC_OK;
}

epc_status_t epc660_cfg_set_integration_time_us(uint16_t tint_us)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Calculates and sets register values to achieve the desired integration
	 * time in microseconds. Uses the formula from datasheet equation 7:
	 *   tINT = (reg0x85+1)/96MHz * (reg0xA2:A3+1) * reg0xA0:A1
	 *
	 * This function reads the current modulation divider and computes
	 * optimal multiplier and base_count values.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read current modulation divider from register 0x85
	 *    - epc_i2c_read(0x85, &mod_div, EPC_DIRECT)
	 *
	 * 2. Calculate the modulation period in nanoseconds
	 *    - t_mod_ns = (mod_div + 1) * 1000 / 96  // ~10.4ns for div=0
	 *
	 * 3. Calculate required integration counts
	 *    - Target: tint_us * 1000 ns = t_mod_ns * (base+1) * multiplier
	 *    - Solve for optimal (base_count, multiplier) pair
	 *    - Prefer larger base_count with smaller multiplier for less noise
	 *
	 * 4. Validate result is achievable
	 *    - base_count: 0 to 65535 (16-bit)
	 *    - multiplier: 0 to 65535 (16-bit)
	 *    - If tint_us too large, return EPC_ERR_PARAM
	 *
	 * 5. Write registers:
	 *    - 0xA0 = multiplier low byte
	 *    - 0xA1 = multiplier high byte
	 *    - 0xA2 = base_count low byte
	 *    - 0xA3 = base_count high byte
	 *
	 * 6. Return EPC_OK on success
	 *
	 * EXAMPLE CALCULATION (12MHz modulation, 100us target):
	 *   mod_div = 1, t_mod = 20.8ns
	 *   Target = 100,000ns
	 *   If base_count = 479, multiplier = 10:
	 *   Actual = 20.8 * 480 * 10 = 99,840ns ≈ 100us
	 */

	epc_status_t status;
	uint8_t mod_div = 0;

	// Read the current modulation divider
    if ((status = epc_i2c_read(EPC_REG_MOD_DIV, &mod_div, EPC_DIRECT)) != EPC_OK) {
        return status;
    }
    /* 2) Compute required product P = (base+1) * multiplier */
    double P = (double)tint_us * 96.0 / (double)(mod_div + 1);

    /* Validate achievable range */
	const double MAX_P = 65536.0 * 1023.0; // 16-bit base and 10-bit multiplier
    if (P < 1.0 || P > MAX_P) {
        return EPC_ERR_PARAM;
    }

    /* 3) Prefer small multiplier / large base_count: try multiplier=1..65535 */
    /*    Constraint: (base_count + 1) must be divisible by 4 per datasheet */
	uint16_t selected_mult = 0;
    uint16_t selected_base = 0;
	for (uint32_t m = 1; m <= 1023; ++m) {
        uint32_t base_plus_one = (uint32_t)(P / (double)m + 0.5); // rounded
        /* Round to nearest multiple of 4 (integration length+1 must be div by 4) */
        base_plus_one = ((base_plus_one + 2) / 4) * 4;
        if (base_plus_one < 4) base_plus_one = 4;
        if (base_plus_one > 65536) continue;
        selected_mult = (uint16_t)m;
        selected_base = (uint16_t)(base_plus_one - 1u);
        break;
    }

    if (selected_mult == 0) {
        return EPC_ERR_PARAM;
    }

	/* 4) Write registers (big-endian in EPC register map: A0/A2 are MSB, A1/A3 are LSB) */
	if ((status = epc_i2c_write(0xA0, (uint8_t)((selected_mult >> 8) & 0xFF), EPC_DIRECT)) != EPC_OK) return status;
	if ((status = epc_i2c_write(0xA1, (uint8_t)(selected_mult & 0xFF), EPC_DIRECT)) != EPC_OK) return status;
	if ((status = epc_i2c_write(0xA2, (uint8_t)((selected_base >> 8) & 0xFF), EPC_DIRECT)) != EPC_OK) return status;
	if ((status = epc_i2c_write(0xA3, (uint8_t)(selected_base & 0xFF), EPC_DIRECT)) != EPC_OK) return status;

    return EPC_OK;
}

epc_status_t epc_set_integration_time_raw(uint16_t multiplier, uint16_t base_count)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Directly writes the integration time multiplier and base count
	 * registers without calculation. Use this when you've pre-computed
	 * the values or for precise control.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Write multiplier low byte to 0xA0
	 *    - epc_i2c_write(0xA0, multiplier & 0xFF, EPC_DIRECT)
	 *
	 * 2. Write multiplier high byte to 0xA1
	 *    - epc_i2c_write(0xA1, (multiplier >> 8) & 0xFF, EPC_DIRECT)
	 *
	 * 3. Write base_count low byte to 0xA2
	 *    - epc_i2c_write(0xA2, base_count & 0xFF, EPC_DIRECT)
	 *
	 * 4. Write base_count high byte to 0xA3
	 *    - epc_i2c_write(0xA3, (base_count >> 8) & 0xFF, EPC_DIRECT)
	 *
	 * 5. Return EPC_OK if all writes succeed
	 */

    epc_status_t status;

    // Write each register individually — multi-byte writes may not
    // auto-increment on the EPC660 RAM page (0x80-0xEF).
    if ((status = epc_i2c_write(0xA1, (uint8_t)(multiplier & 0xFF), EPC_DIRECT)) != EPC_OK) return status;
    if ((status = epc_i2c_write(0xA0, (uint8_t)((multiplier >> 8) & 0xFF), EPC_DIRECT)) != EPC_OK) return status;
    if ((status = epc_i2c_write(0xA3, (uint8_t)(base_count & 0xFF), EPC_DIRECT)) != EPC_OK) return status;
    if ((status = epc_i2c_write(0xA2, (uint8_t)((base_count >> 8) & 0xFF), EPC_DIRECT)) != EPC_OK) return status;

    return EPC_OK;
}

epc_status_t epc660_cfg_set_integration_time2_raw(uint16_t base_count)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Sets the secondary integration time for dual-integration (HDR) mode.
	 * In HDR mode, even rows use the primary integration time (0xA0-A3)
	 * and odd rows use this secondary time (0x9E-9F).
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Write base_count low byte to 0x9E
	 *    - epc_i2c_write(0x9E, base_count & 0xFF, EPC_DIRECT)
	 *
	 * 2. Write base_count high byte to 0x9F
	 *    - epc_i2c_write(0x9F, (base_count >> 8) & 0xFF, EPC_DIRECT)
	 *
	 * 3. Return EPC_OK if both writes succeed
	 *
	 * NOTE: This register pair only affects timing when dual-integration
	 *       mode is enabled via epc660_cfg_set_dual_integration_mode(1).
	 *       The multiplier from 0xA0:A1 is shared with the secondary time.
	 */
	uint8_t data[2];
    data[0] = (uint8_t)(base_count & 0xFF);        // A2 low
    data[1] = (uint8_t)((base_count >> 8) & 0xFF); // A3 high

	return epc_i2c_write_multi(0x9E, data, 2);
}

/*
 * =============================================================================
 * MEASUREMENT MODE CONFIGURATION
 * =============================================================================
 */

epc_status_t epc_set_measurement_mode(epc_measurement_mode_t mode)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Configures the acquisition mode by writing to register 0x92.
	 * This determines whether the chip captures 4 DCS, 2 DCS, or
	 * grayscale images, and whether special modes like HDR are active.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Write mode value directly to register 0x92
	 *    - epc_i2c_write(0x92, (uint8_t)mode, EPC_DIRECT)
	 *
	 * 2. Return status from I2C write
	 *
	 * MODE VALUES (from enum):
	 *   0x34 = 4-DCS TOF (default, best accuracy)
	 *   0x1C = 2-DCS TOF (faster, noisier)
	 *   0xC4 = Grayscale only
	 *   0x3C = 4-DCS dual integration (HDR)
	 *   0x14 = 2-DCS dual phase (motion blur reduction)
	 *
	 * NOTE: Some modes require additional register configuration.
	 *       Grayscale mode needs 0x3C set via epc660_cfg_set_grayscale_mode().
	 *       Dual modes need 0x94 bit 7 set.
	 */
	epc_status_t status;
	if ((status = epc_i2c_write(EPC_REG_MEASUREMENT_MODE, (uint8_t)mode, EPC_DIRECT)) != EPC_OK) return status;
	return EPC_OK;
}

epc_status_t epc_set_grayscale_mode(epc_grayscale_mode_t gs_mode)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Configures the grayscale sub-mode in register 0x3C.
	 * Only effective when measurement mode is set to grayscale (0xC4).
	 * Controls whether LED is active and if modulated or DC.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Write grayscale mode value to register 0x3C
	 *    - epc_i2c_write(0x3C, (uint8_t)gs_mode, EPC_DIRECT)
	 *
	 * 2. Return status from I2C write
	 *
	 * MODE VALUES (from enum):
	 *   0x26 = Ambient only (LED off)
	 *   0x16 = Ambient + DC LED (non-modulated)
	 *   0x06 = Ambient + modulated LED
	 *
	 * NOTE: For temperature sensing, use 0x26 (ambient only) mode
	 *       with special trim register settings.
	 */

	return epc_i2c_write(EPC_REG_GRAYSCALE_MOD_CTRL, (uint8_t)gs_mode, EPC_DIRECT);
}

epc_status_t epc660_cfg_set_auto_run(uint8_t enable)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Enables or disables continuous acquisition mode via register 0xA4.
	 * When enabled, the chip continuously acquires frames without
	 * needing individual shutter triggers.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read current value of register 0xA4
	 *    - epc_i2c_read(0xA4, &reg_val, EPC_DIRECT)
	 *
	 * 2. Modify bit 1 based on enable parameter
	 *    - if (enable) reg_val |= 0x02;   // Set bit 1
	 *    - else        reg_val &= ~0x02;  // Clear bit 1
	 *
	 * 3. Write modified value back to 0xA4
	 *    - epc_i2c_write(0xA4, reg_val, EPC_DIRECT)
	 *
	 * 4. Return status from I2C write
	 *
	 * BEHAVIOR:
	 *   enable=1: Frames acquired continuously, VSYNC pulses for each
	 *   enable=0: Single-shot mode, one SHUTTER = one frame set
	 *
	 * WARNING: In auto-run mode, ensure TCMI capture is ready before
	 *          enabling, otherwise frames may be lost.
	 */

	epc_status_t status;
	uint8_t reg_val;

	// Read current shutter control
	if ((status = epc_i2c_read(EPC_REG_SHUTTER_CTRL, &reg_val, EPC_DIRECT)) != EPC_OK) return status;

	// Modify auto-run bit
	if (enable) reg_val |= 0x02;
	else 		reg_val &= ~0x02;

	// Write back new shutter control
	if ((status = epc_i2c_write(EPC_REG_SHUTTER_CTRL, reg_val, EPC_DIRECT)) != EPC_OK) return status;

	return EPC_OK;
}

epc_status_t epc660_cfg_set_dual_phase_mode(uint8_t enable)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Enables dual-phase acquisition for motion blur reduction.
	 * In this mode, two phase-shifted DCS (0+1 or 2+3) are captured
	 * simultaneously using even/odd rows. Resolution becomes 320x120
	 * but temporal alignment eliminates motion artifacts.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read current value of register 0x94
	 *    - epc_i2c_read(0x94, &reg_val, EPC_DIRECT)
	 *
	 * 2. Modify bit 7 based on enable parameter
	 *    - if (enable) reg_val |= 0x80;   // Set bit 7
	 *    - else        reg_val &= ~0x80;  // Clear bit 7
	 *
	 * 3. Write modified value back to 0x94
	 *    - epc_i2c_write(0x94, reg_val, EPC_DIRECT)
	 *
	 * 4. If enabling, configure modulation table registers:
	 *    - epc_i2c_write(0x22, 0xNN, EPC_DIRECT)  // Phase for even rows
	 *    - epc_i2c_write(0x25, 0xNN, EPC_DIRECT)  // Phase for odd rows
	 *    (Refer to datasheet Table 25 for specific values)
	 *
	 * 5. If disabling, restore default modulation table values
	 *
	 * 6. Return EPC_OK if all writes succeed
	 *
	 * NOTE: Cannot be combined with binning mode.
	 *       Requires post-processing to reconstruct full frame.
	 */

	(void)enable;
	return EPC_OK;
}

epc_status_t epc660_cfg_set_dual_integration_mode(uint8_t enable)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Enables dual-integration (HDR) mode for high dynamic range imaging.
	 * Even rows integrate with primary time (0xA0-A3), odd rows with
	 * secondary time (0x9E-9F). Resolution becomes 320x120 with two
	 * different exposure levels.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read current value of register 0x94
	 *    - epc_i2c_read(0x94, &reg_val, EPC_DIRECT)
	 *
	 * 2. Modify bit 7 based on enable parameter
	 *    - if (enable) reg_val |= 0x80;   // Set bit 7
	 *    - else        reg_val &= ~0x80;  // Clear bit 7
	 *
	 * 3. Write modified value back to 0x94
	 *    - epc_i2c_write(0x94, reg_val, EPC_DIRECT)
	 *
	 * 4. Return status from I2C write
	 *
	 * NOTE: After enabling, set the secondary integration time using
	 *       epc660_cfg_set_integration_time2_raw().
	 *       Typically use a 4:1 or 8:1 ratio between primary and secondary.
	 *       Example: Primary=800us, Secondary=100us for 8x HDR range.
	 */

	(void)enable;
	return EPC_OK;
}

/*
 * =============================================================================
 * ROI, BINNING, AND RESOLUTION CONFIGURATION
 * =============================================================================
 */

epc_status_t epc660_cfg_set_roi(uint16_t x_start, uint16_t y_start,
                                 uint16_t x_end, uint16_t y_end)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Configures Region of Interest to read a sub-window of the sensor.
	 * Reduces readout time proportionally to the number of rows.
	 * The sensor has dual pixel fields (top/bottom), so ROI is defined
	 * for the top half (rows 6-125) and automatically mirrored.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Validate input parameters:
	 *    - x_start >= 4 && x_start <= 322 && x_start is even
	 *    - y_start >= 6 && y_start <= 124 && y_start is even
	 *    - x_end >= 5 && x_end <= 323 && x_end is odd
	 *    - y_end >= 7 && y_end <= 125 && y_end is odd
	 *    - x_end > x_start (at least 6 columns)
	 *    - y_end > y_start (at least 2 rows)
	 *    - If any invalid, return EPC_ERR_PARAM
	 *
	 * 2. Write ROI registers (all 16-bit split across 2 regs):
	 *    - 0x96 = x_start low byte
	 *    - 0x97 = x_start high byte
	 *    - 0x98 = y_start low byte
	 *    - 0x99 = y_start high byte
	 *    - 0x9A = x_end low byte
	 *    - 0x9B = x_end high byte
	 *    - 0x9C = y_end low byte (if exists, check datasheet)
	 *    - 0x9D = y_end high byte
	 *
	 * 3. Return EPC_OK if all writes succeed
	 *
	 * EXAMPLE (center 160x120 ROI):
	 *   x_start=84, y_start=36, x_end=243, y_end=95
	 *   Results in 160 columns x 120 rows (60 per half)
	 *
	 * NOTE: ROI affects frame size for DMA buffer calculation.
	 *       New frame size = (x_end - x_start + 1) * (y_end - y_start + 1) * 2
	 *       (doubled for top+bottom pixel fields)
	 */

	(void)x_start;
	(void)y_start;
	(void)x_end;
	(void)y_end;
	return EPC_OK;
}

epc_status_t epc660_cfg_reset_roi(void)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Resets Region of Interest to full frame (320x240 pixels).
	 * This is a convenience wrapper around epc660_cfg_set_roi()
	 * with the default full-frame coordinates.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Call set_roi with default full-frame values:
	 *    - return epc660_cfg_set_roi(4, 6, 323, 125);
	 *
	 * FULL FRAME DIMENSIONS:
	 *   - Columns: 4 to 323 (320 columns)
	 *   - Rows: 6 to 125 (120 rows per half, 240 total)
	 */

	return EPC_OK;
}

epc_status_t epc660_cfg_set_binning(epc_binning_mode_t binning)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Configures pixel binning mode in register 0x94 bits 0-1.
	 * Binning combines adjacent pixels for increased sensitivity
	 * at reduced spatial resolution.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read current value of register 0x94
	 *    - epc_i2c_read(0x94, &reg_val, EPC_DIRECT)
	 *
	 * 2. Clear bits 0-1, preserve other bits
	 *    - reg_val &= ~0x03;
	 *
	 * 3. Set new binning mode
	 *    - reg_val |= (binning & 0x03);
	 *
	 * 4. Write modified value back to 0x94
	 *    - epc_i2c_write(0x94, reg_val, EPC_DIRECT)
	 *
	 * 5. Return status from I2C write
	 *
	 * BINNING EFFECTS:
	 *   NONE:       320x240, 1x sensitivity (default)
	 *   HORIZONTAL: 160x240, 2x sensitivity
	 *   VERTICAL:   320x120, 2x sensitivity
	 *   BOTH:       160x120, 4x sensitivity
	 *
	 * WARNING: Cannot be used with dual-phase or dual-integration modes.
	 */

	(void)binning;
	return EPC_OK;
}

epc_status_t epc660_cfg_set_row_reduction(epc_row_reduction_t reduction)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Configures row reduction (skip N rows) in register 0x94 bits 2-3.
	 * Increases frame rate by reading fewer rows, at the cost of
	 * vertical resolution.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read current value of register 0x94
	 *    - epc_i2c_read(0x94, &reg_val, EPC_DIRECT)
	 *
	 * 2. Clear bits 2-3, preserve other bits
	 *    - reg_val &= ~0x0C;
	 *
	 * 3. Set new reduction mode
	 *    - reg_val |= (reduction & 0x0C);
	 *
	 * 4. Write modified value back to 0x94
	 *    - epc_i2c_write(0x94, reg_val, EPC_DIRECT)
	 *
	 * 5. Return status from I2C write
	 *
	 * REDUCTION EFFECTS:
	 *   NONE: 240 rows (default)
	 *   2:    120 rows
	 *   4:    60 rows
	 *   8:    30 rows
	 *
	 * NOTE: Row reduction does NOT combine pixels (unlike binning).
	 *       Skipped rows are simply not read out.
	 */

	(void)reduction;
	return EPC_OK;
}

epc_status_t epc660_cfg_set_col_reduction(epc_col_reduction_t reduction)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Configures column reduction (skip every 2nd column) in register 0x94 bit 4.
	 * Increases frame rate by reading fewer columns.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read current value of register 0x94
	 *    - epc_i2c_read(0x94, &reg_val, EPC_DIRECT)
	 *
	 * 2. Clear bit 4, preserve other bits
	 *    - reg_val &= ~0x10;
	 *
	 * 3. Set new reduction mode
	 *    - reg_val |= (reduction & 0x10);
	 *
	 * 4. Write modified value back to 0x94
	 *    - epc_i2c_write(0x94, reg_val, EPC_DIRECT)
	 *
	 * 5. Return status from I2C write
	 *
	 * REDUCTION EFFECTS:
	 *   NONE: 320 columns (default)
	 *   2:    160 columns
	 */

	(void)reduction;
	return EPC_OK;
}

/*
 * =============================================================================
 * LED/ILLUMINATION DRIVER CONFIGURATION
 * =============================================================================
 */

epc_status_t epc660_cfg_set_led_output(epc_led_output_t output)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Selects which LED driver output pin to use (LED or LED2).
	 * Only one should be used at a time to avoid conflicts.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read current value of register 0x90
	 *    - epc_i2c_read(0x90, &reg_val, EPC_DIRECT)
	 *
	 * 2. Modify bit 1 (LED2 enable) based on output parameter
	 *    - if (output == EPC_LED_PUSH_PULL) reg_val |= 0x02;   // Enable LED2
	 *    - else                              reg_val &= ~0x02;  // Use LED
	 *
	 * 3. Write modified value back to 0x90
	 *    - epc_i2c_write(0x90, reg_val, EPC_DIRECT)
	 *
	 * 4. Return status from I2C write
	 *
	 * PIN CHARACTERISTICS:
	 *   LED (pin 12):  Open-drain, sink up to 200mA @ Tj<85°C
	 *   LED2 (pin 37): Push-pull, 50mA max, better timing symmetry
	 *
	 * WARNING: Do not enable both outputs simultaneously.
	 *          Ensure external LED driver is compatible with chosen polarity.
	 */

	(void)output;
	return EPC_OK;
}

epc_status_t epc660_cfg_set_led_polarity(uint8_t active_high)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Sets the polarity of the LED/LED2 output signal.
	 * This must match the external LED driver's input requirements.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read current value of register 0x90
	 *    - epc_i2c_read(0x90, &reg_val, EPC_DIRECT)
	 *
	 * 2. Modify bit 0 (polarity) based on active_high parameter
	 *    - if (active_high) reg_val |= 0x01;   // Active high
	 *    - else             reg_val &= ~0x01;  // Active low
	 *
	 * 3. Write modified value back to 0x90
	 *    - epc_i2c_write(0x90, reg_val, EPC_DIRECT)
	 *
	 * 4. Return status from I2C write
	 *
	 * NOTE: Common LED drivers (like the one in epc660 eval kit)
	 *       typically use active-high input.
	 */

	(void)active_high;
	return EPC_OK;
}

epc_status_t epc660_cfg_set_led_preheat(uint8_t enable)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Enables LED pre-heating for improved ambient light suppression.
	 * When enabled, the LED modulates for ~33us before integration starts,
	 * which improves ALS performance by approximately 20%.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read current value of register 0x90
	 *    - epc_i2c_read(0x90, &reg_val, EPC_DIRECT)
	 *
	 * 2. Modify bit 3 (preheat enable) based on enable parameter
	 *    - if (enable) reg_val |= 0x08;   // Enable preheat
	 *    - else        reg_val &= ~0x08;  // Disable preheat
	 *
	 * 3. Write modified value back to 0x90
	 *    - epc_i2c_write(0x90, reg_val, EPC_DIRECT)
	 *
	 * 4. If enabling, set preheat timing register
	 *    - epc_i2c_write(0xAB, 0x01, EPC_DIRECT)
	 *
	 * 5. Return EPC_OK if all writes succeed
	 *
	 * NOTE: Preheat is recommended for outdoor or high-ambient applications.
	 *       It adds a small amount to total measurement time (~33us).
	 */

	(void)enable;
	return EPC_OK;
}

/*
 * =============================================================================
 * DLL (DELAY LINE) CONFIGURATION
 * =============================================================================
 */

epc_status_t epc660_cfg_set_dll_enable(uint8_t enable)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Enables or disables manual DLL (Delay Locked Loop) control.
	 * The DLL allows phase-shifting the LED modulation relative to
	 * the demodulation clock, which can optimize accuracy for specific
	 * distance ranges.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Write DLL enable register 0xAE
	 *    - if (enable) epc_i2c_write(0xAE, 0x04, EPC_DIRECT)
	 *    - else        epc_i2c_write(0xAE, 0x00, EPC_DIRECT)
	 *
	 * 2. Return status from I2C write
	 *
	 * NOTE: After enabling DLL, set the desired delay using
	 *       epc660_cfg_set_dll_coarse() and epc660_cfg_set_dll_fine().
	 *       DLL is useful for optimizing measurements at specific depths.
	 */

	(void)enable;
	return EPC_OK;
}

epc_status_t epc660_cfg_set_dll_coarse(uint8_t steps)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Sets the DLL coarse delay in steps. Each step is approximately
	 * 2.1ns, corresponding to about 30cm of apparent distance shift.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Validate steps is within range
	 *    - if (steps > 49) return EPC_ERR_PARAM
	 *
	 * 2. Write steps value to register 0x73
	 *    - epc_i2c_write(0x73, steps, EPC_DIRECT)
	 *
	 * 3. Return status from I2C write
	 *
	 * DLL RANGE:
	 *   0 steps  = 0ns delay
	 *   49 steps = ~103ns delay (~15.5m distance offset at 12MHz mod)
	 *
	 * NOTE: DLL must be enabled first via epc660_cfg_set_dll_enable(1).
	 */

	(void)steps;
	return EPC_OK;
}

epc_status_t epc660_cfg_set_dll_fine(uint16_t fine_value)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Sets the DLL fine delay for precise phase tuning.
	 * This provides finer adjustment than the coarse delay.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Write fine_value low byte to register 0x71
	 *    - epc_i2c_write(0x71, fine_value & 0xFF, EPC_DIRECT)
	 *
	 * 2. Write fine_value high byte to register 0x72
	 *    - epc_i2c_write(0x72, (fine_value >> 8) & 0xFF, EPC_DIRECT)
	 *
	 * 3. Return EPC_OK if both writes succeed
	 *
	 * NOTE: Fine delay resolution and range depend on the PLL frequency.
	 *       Refer to datasheet for exact timing specifications.
	 */

	(void)fine_value;
	return EPC_OK;
}

/*
 * =============================================================================
 * TCMI (PARALLEL DATA OUTPUT) CONFIGURATION
 * =============================================================================
 */

epc_status_t epc660_cfg_set_tcmi_format(uint8_t format)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Configures the TCMI data output format in register 0xCB.
	 * The EPC660 can output 12-bit data in several formats depending
	 * on the receiving MCU's capabilities.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Write format value to register 0xCB
	 *    - epc_i2c_write(0xCB, (uint8_t)format, EPC_DIRECT)
	 *
	 * 2. Return status from I2C write
	 *
	 * NOTE: STM32 DCMI typically works with 8-bit or 12-bit parallel.
	 *       For 12-bit data on 8-bit DCMI, use MSB_LSB format.
	 */

	return epc_i2c_write(0xCB, format, EPC_DIRECT);
}

epc_status_t epc_set_dclk_freq(epc_dclk_freq_t freq)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Sets the TCMI data clock (DCLK) frequency by configuring the
	 * clock divider in register 0x89. DCLK drives pixel data output.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Write divider value to register 0x89
	 *    - epc_i2c_write(0x89, (uint8_t)freq, EPC_DIRECT)
	 *
	 * 2. If freq results in DCLK < 13MHz, enable HSYNC stretching
	 *    - if (freq > EPC_DCLK_16MHZ) epc660_cfg_set_hsync_stretch(1)
	 *
	 * 3. Return EPC_OK on success
	 *
	 * FREQUENCY OPTIONS (96MHz PLL / (divider + 1)):
	 *   0x01 = 48MHz (fastest)
	 *   0x02 = 32MHz
	 *   0x03 = 24MHz (default)
	 *   0x04 = 19.2MHz
	 *   0x05 = 16MHz
	 *   0x07 = 12MHz (HSYNC stretch recommended)
	 *
	 * NOTE: MCU's DCMI peripheral must support the selected frequency.
	 *       Faster DCLK = higher frame rate but more demanding on MCU.
	 */

	return epc_i2c_write(EPC_REG_DCLK_CTRL, (uint8_t)freq, EPC_DIRECT);
}

epc_status_t epc660_cfg_set_tcmi_polarity(uint8_t vsync_active_high,
                                           uint8_t hsync_active_high,
                                           uint8_t dclk_active_high,
                                           uint8_t sat_on_xsync)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Configures the polarity of TCMI synchronization signals in
	 * register 0xCC. Must match the MCU's DCMI configuration.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Build register value from parameters
	 *    - reg_val = 0x00
	 *    - if (vsync_active_high) reg_val |= 0x01;  // Bit 0
	 *    - if (hsync_active_high) reg_val |= 0x02;  // Bit 1
	 *    - if (dclk_active_high)  reg_val |= 0x04;  // Bit 2
	 *    - if (sat_on_xsync)      reg_val |= 0x08;  // Bit 3
	 *
	 * 2. Write to register 0xCC
	 *    - epc_i2c_write(0xCC, reg_val, EPC_DIRECT)
	 *
	 * 3. Return status from I2C write
	 *
	 * STM32 DCMI TYPICAL SETTINGS:
	 *   VSYNC: Active high (1)
	 *   HSYNC: Active high (1)
	 *   DCLK:  Rising edge (1) - data sampled on rising edge
	 *
	 * NOTE: These settings must match DCMI_InitTypeDef configuration:
	 *   VSPolarity, HSPolarity, PCKPolarity
	 */

	(void)vsync_active_high;
	(void)hsync_active_high;
	(void)dclk_active_high;
	(void)sat_on_xsync;
	return EPC_OK;
}

epc_status_t epc_set_hsync_stretch(uint8_t enable)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Enables HSYNC stretching for slower DCLK frequencies.
	 * Required when DCLK < 13MHz to prevent readout from rolling over
	 * into the next ADC conversion cycle.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read current value of register 0x91
	 *    - epc_i2c_read(0x91, &reg_val, EPC_DIRECT)
	 *
	 * 2. Modify bit 6 (HSYNC stretch enable) based on enable parameter
	 *    - if (enable) reg_val |= 0x40;   // Set bit 6
	 *    - else        reg_val &= ~0x40;  // Clear bit 6
	 *
	 * 3. Write modified value back to 0x91
	 *    - epc_i2c_write(0x91, reg_val, EPC_DIRECT)
	 *
	 * 4. Return status from I2C write
	 *
	 * NOTE: HSYNC stretching adds ~2us per ADC conversion.
	 *       This increases total frame time but ensures data integrity.
	 *       Automatically enabled by epc660_cfg_set_dclk_freq() for slow clocks.
	 */

	if (enable)
	{
		return epc_i2c_write(0x91, 0x43, EPC_DIRECT);  // Enable HSYNC stretch
	}
	else
	{
		return epc_i2c_write(0x91, 0x03, EPC_DIRECT);  // Disable HSYNC stretch
	}
}

/*
 * =============================================================================
 * EXTERNAL MODULATION CLOCK CONFIGURATION
 * =============================================================================
 */

epc_status_t epc660_cfg_set_external_modclk(uint8_t enable)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Enables use of external modulation clock (MODCLK input pin)
	 * instead of the internal PLL-generated clock. Useful for
	 * multi-camera systems using FDMA (Frequency Division Multiple Access).
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read current value of register 0x80
	 *    - epc_i2c_read(0x80, &reg_val, EPC_DIRECT)
	 *
	 * 2. Modify bit 6 (external MODCLK enable) based on enable parameter
	 *    - if (enable) reg_val |= 0x40;   // Set bit 6
	 *    - else        reg_val &= ~0x40;  // Clear bit 6
	 *
	 * 3. Write modified value back to 0x80
	 *    - epc_i2c_write(0x80, reg_val, EPC_DIRECT)
	 *
	 * 4. Return status from I2C write
	 *
	 * EXTERNAL CLOCK REQUIREMENTS:
	 *   - Frequency: Up to 96MHz
	 *   - Duty cycle: 50% ± 5%
	 *   - Applied to MODCLK pin (pin XX)
	 *
	 * NOTE: When using external clock, the modulation frequency is
	 *       determined by the external source, not register 0x85.
	 */

	(void)enable;
	return EPC_OK;
}

/*
 * =============================================================================
 * TEMPERATURE SENSOR CONFIGURATION
 * =============================================================================
 */

epc_status_t epc660_cfg_temp_sensor_init(void)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Initializes the temperature sensor subsystem for reading.
	 * The EPC660 has 4 on-chip temperature sensors. Before reading,
	 * certain trim registers must be modified, and factory calibration
	 * values must be loaded from EEPROM.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Save current trim register values (to restore later):
	 *    - epc_i2c_read(0xD3, &_saved_trim_d3, EPC_DIRECT)
	 *    - epc_i2c_read(0xD5, &_saved_trim_d5, EPC_DIRECT)
	 *    - epc_i2c_read(0xDA, &_saved_trim_da, EPC_DIRECT)
	 *    - epc_i2c_read(0xDC, &_saved_trim_dc, EPC_DIRECT)
	 *
	 * 2. Modify trim registers for temperature sensing mode:
	 *    - Specific values per datasheet section 10
	 *    - epc_i2c_write(0xD3, 0xXX, EPC_DIRECT)
	 *    - epc_i2c_write(0xD5, 0xXX, EPC_DIRECT)
	 *    - epc_i2c_write(0xDA, 0xXX, EPC_DIRECT)
	 *    - epc_i2c_write(0xDC, 0xXX, EPC_DIRECT)
	 *
	 * 3. Read factory calibration values from EEPROM (0xE8-0xEE):
	 *    - For each of 4 sensors, read 2-byte calibration offset
	 *    - epc_i2c_read(0xE8+i*2, &cal_low, EPC_INDIRECT)
	 *    - epc_i2c_read(0xE9+i*2, &cal_high, EPC_INDIRECT)
	 *    - _temp_cal_offset[i] = (cal_high << 8) | cal_low
	 *
	 * 4. Return EPC_OK on success
	 *
	 * NOTE: After temperature reading, call epc660_cfg_temp_sensor_restore()
	 *       to restore normal imaging operation.
	 */

	return EPC_OK;
}

epc_status_t epc660_cfg_temp_read_raw(uint16_t raw_temps[4])
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Reads all four temperature sensors and returns raw ADC values.
	 * The chip must be in grayscale mode with temperature sensing
	 * configuration (set by epc660_cfg_temp_sensor_init()).
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Validate input pointer
	 *    - if (raw_temps == NULL) return EPC_ERR_PARAM
	 *
	 * 2. Set measurement mode to grayscale (if not already)
	 *    - epc660_cfg_set_measurement_mode(EPC_MODE_GRAYSCALE)
	 *    - epc660_cfg_set_grayscale_mode(EPC_GS_AMBIENT_ONLY)
	 *
	 * 3. Trigger a single measurement
	 *    - epc_i2c_write(0xA4, 0x01, EPC_DIRECT)
	 *
	 * 4. Wait for measurement complete
	 *    - Poll register or use fixed delay (~frame time)
	 *    - epc_delay_ms(10)  // Adjust based on integration time
	 *
	 * 5. Read temperature sensor registers (0x60-0x67):
	 *    - For each of 4 sensors (i=0..3):
	 *      - epc_i2c_read(0x60 + i*2, &low_byte, EPC_DIRECT)
	 *      - epc_i2c_read(0x61 + i*2, &high_byte, EPC_DIRECT)
	 *      - raw_temps[i] = (high_byte << 8) | low_byte
	 *
	 * 6. Return EPC_OK on success
	 *
	 * SENSOR LOCATIONS (per datasheet):
	 *   Sensor 0: Top-left corner
	 *   Sensor 1: Top-right corner
	 *   Sensor 2: Bottom-left corner
	 *   Sensor 3: Bottom-right corner
	 *
	 * NOTE: Temperature mode sensitivity is 2.5x lower than normal grayscale.
	 */

	(void)raw_temps;
	return EPC_OK;
}

float epc660_cfg_temp_to_celsius(uint16_t raw_value, int16_t cal_offset)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Converts a raw temperature sensor reading to degrees Celsius.
	 * Uses the formula from datasheet section 10:
	 *   Temp_C = (raw - 0x2000) * 0.134 + offset
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Calculate temperature using formula:
	 *    - int32_t delta = (int32_t)raw_value - 0x2000;
	 *    - float temp_c = (float)delta * 0.134f + (float)cal_offset;
	 *
	 * 2. Return temp_c
	 *
	 * EXAMPLE:
	 *   raw_value = 0x2100 (8448)
	 *   cal_offset = 25
	 *   delta = 8448 - 8192 = 256
	 *   temp_c = 256 * 0.134 + 25 = 59.3°C
	 *
	 * NOTE: The calibration offset is chip-specific and read from EEPROM
	 *       during epc660_cfg_temp_sensor_init().
	 */

	(void)raw_value;
	(void)cal_offset;
	return 0.0f;
}

epc_status_t epc660_cfg_temp_sensor_restore(void)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Restores the trim registers to their original values after
	 * temperature reading, allowing normal imaging operation to resume.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Restore saved trim register values:
	 *    - epc_i2c_write(0xD3, _saved_trim_d3, EPC_DIRECT)
	 *    - epc_i2c_write(0xD5, _saved_trim_d5, EPC_DIRECT)
	 *    - epc_i2c_write(0xDA, _saved_trim_da, EPC_DIRECT)
	 *    - epc_i2c_write(0xDC, _saved_trim_dc, EPC_DIRECT)
	 *
	 * 2. Optionally restore measurement mode to previous setting
	 *    - epc660_cfg_set_measurement_mode(previous_mode)
	 *
	 * 3. Return EPC_OK if all writes succeed
	 *
	 * NOTE: Always call this after temperature reading to ensure
	 *       normal TOF/imaging operation is not affected.
	 */

	return EPC_OK;
}

/*
 * =============================================================================
 * SATURATION THRESHOLD CONFIGURATION
 * =============================================================================
 */

epc_status_t epc660_cfg_set_saturation_threshold(uint8_t threshold)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Sets the software saturation detection threshold in register 0xAF.
	 * Lower values make saturation detection more sensitive.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Write threshold value to register 0xAF
	 *    - epc_i2c_write(0xAF, threshold, EPC_DIRECT)
	 *
	 * 2. Return status from I2C write
	 *
	 * DEFAULT VALUE:
	 *   0x39 for 125us integration time
	 *   Adjust proportionally for other integration times
	 *
	 * NOTE: Saturation occurs when a pixel receives too much light.
	 *       Saturated pixels give unreliable depth readings.
	 *       The saturation flag can be output on XSYNC_SAT pin or
	 *       force pixel data to 0xFFF (see set_saturation_output).
	 */

	(void)threshold;
	return EPC_OK;
}

epc_status_t epc_set_software_saturation_flag(uint8_t force_fff)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Configures how saturation is indicated in the output data.
	 * Options include outputting a flag on XSYNC_SAT pin and/or
	 * forcing saturated pixel data to 0xFFF.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. Read current value of register 0xCC
	 *    - epc_i2c_read(0xCC, &reg_val, EPC_DIRECT)
	 *
	 * 2. Modify bits 6-7 based on parameters
	 *    - reg_val &= ~0xC0;  // Clear bits 6-7
	 *    - if (force_fff)  reg_val |= 0x80;  // Bit 7: Force 0xFFF
	 *    - if (use_xsync)  reg_val |= 0x40;  // Bit 6: Output on XSYNC_SAT
	 *
	 * 3. Write modified value back to 0xCC
	 *    - epc_i2c_write(0xCC, reg_val, EPC_DIRECT)
	 *
	 * 4. Return status from I2C write
	 *
	 * SATURATION HANDLING OPTIONS:
	 *   force_fff=0, use_xsync=0: No indication (raw saturated values)
	 *   force_fff=0, use_xsync=1: Flag on XSYNC_SAT pin (per pixel)
	 *   force_fff=1, use_xsync=0: Force DATA=0xFFF when saturated
	 *   force_fff=1, use_xsync=1: Both methods active
	 *
	 * NOTE: Forcing 0xFFF makes saturation easy to detect in software
	 *       without needing to monitor an extra GPIO pin.
	 */

	epc_status_t status;
	uint8_t reg_val;

	if ((status = epc_i2c_read(EPC_REG_TCMI_POLARITY, &reg_val, EPC_DIRECT)) != EPC_OK) return status;

	reg_val &= ~0x80;
	if (force_fff)	reg_val |= 0x80;

	if ((status = epc_i2c_write(EPC_REG_TCMI_POLARITY, reg_val, EPC_DIRECT)) != EPC_OK) return status;
	return EPC_OK;
}

/*
 * =============================================================================
 * ROLLING DCS MODE CONFIGURATION
 * =============================================================================
 */

epc_status_t epc660_cfg_set_rolling_mode(uint8_t enable)
{
	/*
	 * HIGH-LEVEL DESCRIPTION:
	 * -----------------------
	 * Enables enhanced rolling DCS frame mode for maximum frame rate.
	 * In rolling mode, integration and readout are interleaved,
	 * allowing frame rates up to 158 fps at full resolution.
	 *
	 * PSEUDOCODE:
	 * -----------
	 * 1. If enabling rolling mode:
	 *    a. Set rolling mode enable bit
	 *       - Read register 0xXX
	 *       - Set appropriate bit
	 *       - Write back
	 *
	 *    b. Configure timing parameters per datasheet section 11.6-11.7:
	 *       - Adjust sequencer timing
	 *       - Configure row timing overlap
	 *       - Set appropriate delays
	 *
	 * 2. If disabling rolling mode:
	 *    - Clear rolling mode enable bit
	 *    - Restore default timing parameters
	 *
	 * 3. Return EPC_OK on success
	 *
	 * ROLLING MODE BENEFITS:
	 *   - Up to 158 fps at 320x240 (vs ~60 fps non-rolling)
	 *   - More efficient use of sensor time
	 *
	 * ROLLING MODE DRAWBACKS:
	 *   - Slightly higher noise due to integration overlap
	 *   - More complex DCS timing
	 *   - May require adjusted sequencer code
	 *
	 * NOTE: Refer to datasheet sections 11.6 and 11.7 for detailed
	 *       register settings required for rolling mode.
	 */

	(void)enable;
	return EPC_OK;
}

/* ===== AI-GENERATED BLOCK END ===== */


