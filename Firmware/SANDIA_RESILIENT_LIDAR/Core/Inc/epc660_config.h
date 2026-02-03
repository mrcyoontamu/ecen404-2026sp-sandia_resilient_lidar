/*
 * epc660_config.h
 *
 *  Created on: Jan 22, 2026
 *      Author: unrea
 */

#ifndef INC_EPC660_CONFIG_H_
#define INC_EPC660_CONFIG_H_

#include "epc660_platform.h"

/* ===== AI-GENERATED BLOCK START ===== */
/*
 * -----------------------------------------------------------------------------
 * EPC660 CONFIGURATION TYPES & ENUMERATIONS
 * These types define valid configuration options per the datasheet.
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Measurement mode selection (register 0x92).
 *
 * Defines DCS acquisition modes per datasheet section 9.1 and 11.2.
 */
typedef enum {
    EPC_MODE_4DCS_TOF       = 0x34,  /**< 4-DCS TOF mode (default, best accuracy) */
    EPC_MODE_2DCS_TOF       = 0x1C,  /**< 2-DCS TOF mode (faster, higher noise) */
    EPC_MODE_GRAYSCALE      = 0xC4,  /**< Grayscale/ambient-light only */
    EPC_MODE_4DCS_DUAL_INT  = 0x3C,  /**< 4-DCS dual integration time (HDR) */
    EPC_MODE_2DCS_DUAL_INT  = 0x1C,  /**< 2-DCS dual integration time (HDR) */
    EPC_MODE_2DCS_DUAL_PHASE= 0x14   /**< 2-DCS dual phase (motion blur reduction) */
} epc_measurement_mode_t;

/**
 * @brief Grayscale sub-mode selection (register 0x3C).
 *
 * Per datasheet Table 27.
 */
typedef enum {
    EPC_GS_AMBIENT_ONLY          = 0x26, /**< Ambient light only, no LED */
    EPC_GS_AMBIENT_DC_LED        = 0x16, /**< Ambient + non-modulated LED */
    EPC_GS_AMBIENT_MODULATED_LED = 0x06  /**< Ambient + modulated LED */
} epc_grayscale_mode_t;

/**
 * @brief LED driver output selection (register 0x90).
 *
 * Choose between open-drain LED pin or push-pull LED2 pin.
 */
typedef enum {
    EPC_LED_OPEN_DRAIN = 0, /**< Use LED pin (open-drain, up to 200mA) */
    EPC_LED_PUSH_PULL  = 1  /**< Use LED2 pin (push-pull, up to 50mA) */
} epc_led_output_t;

/**
 * @brief Pixel binning mode (register 0x94 bits 0-1).
 *
 * Per datasheet section 8.4 / Figure 41.
 */
typedef enum {
    EPC_BINNING_NONE       = 0x00, /**< No binning (320x240) */
    EPC_BINNING_HORIZONTAL = 0x01, /**< Horizontal binning (160x240, 2x sensitivity) */
    EPC_BINNING_VERTICAL   = 0x02, /**< Vertical binning (320x120, 2x sensitivity) */
    EPC_BINNING_BOTH       = 0x03  /**< Both (160x120, 4x sensitivity) */
} epc_binning_mode_t;

/**
 * @brief Row reduction factor (register 0x94 bits 2-3).
 *
 * Read every Nth row to increase frame rate.
 */
typedef enum {
    EPC_ROW_REDUCTION_NONE = 0x00, /**< All rows (240 rows) */
    EPC_ROW_REDUCTION_2    = 0x04, /**< Every 2nd row (120 rows) */
    EPC_ROW_REDUCTION_4    = 0x08, /**< Every 4th row (60 rows) */
    EPC_ROW_REDUCTION_8    = 0x0C  /**< Every 8th row (30 rows) */
} epc_row_reduction_t;

/**
 * @brief Column reduction (register 0x94 bit 4).
 *
 * Read every 2nd column.
 */
typedef enum {
    EPC_COL_REDUCTION_NONE = 0x00, /**< All columns (320 cols) */
    EPC_COL_REDUCTION_2    = 0x10  /**< Every 2nd column (160 cols) */
} epc_col_reduction_t;

/**
 * @brief TCMI data format (register 0xCB).
 *
 * Per datasheet section 6.4.
 */
typedef enum {
    EPC_TCMI_12BIT       = 0x00, /**< 12-bit mode, all DATA[11:0] used */
    EPC_TCMI_8BIT_MSB    = 0x01, /**< 8-bit mode, DATA[11:4] only */
    EPC_TCMI_MSB_LSB     = 0x02, /**< 8-bit split: MSB first, LSB second */
    EPC_TCMI_LSB_MSB     = 0x03  /**< 8-bit split: LSB first, MSB second */
} epc_tcmi_format_t;

/**
 * @brief DCLK frequency divider (register 0x89).
 *
 * PLL output (96MHz) divided to get DCLK.
 */
typedef enum {
    EPC_DCLK_48MHZ = 0x02, /**< 96/2 = 48MHz (max) */
    EPC_DCLK_32MHZ = 0x03, /**< 96/3 = 32MHz */
    EPC_DCLK_24MHZ = 0x04, /**< 96/4 = 24MHz (default) */
    EPC_DCLK_19MHZ = 0x05, /**< 96/5 = 19.2MHz */
    EPC_DCLK_16MHZ = 0x06, /**< 96/6 = 16MHz */
    EPC_DCLK_12MHZ = 0x08  /**< 96/8 = 12MHz */
} epc_dclk_freq_t;

/*
 * -----------------------------------------------------------------------------
 * MODULATION & TIMING CONFIGURATION
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Set the modulation frequency for TOF measurement.
 *
 * The LED modulation frequency determines unambiguity range:
 *   - 24MHz -> 6.25m
 *   - 12MHz -> 12.5m (default)
 *   - 6MHz  -> 25m
 *   - 3MHz  -> 50m
 *   - 1.5MHz -> 100m
 *
 * Controlled via register 0x85 (mod_clk divider).
 *
 * @param divider Divider value: 0=24MHz, 1=12MHz, 3=6MHz, 7=3MHz, 15=1.5MHz
 * @return EPC_OK on success, EPC_ERR_PARAM if invalid divider.
 */
epc_status_t epc660_cfg_set_modulation_divider(uint8_t divider);

/**
 * @brief Set integration time for TOF/grayscale measurements.
 *
 * Integration time formula (datasheet eq. 7):
 *   tINT = (reg0x85+1)/96MHz * (reg0xA2:A3+1) * reg0xA0:A1
 *
 * This function accepts integration time in microseconds and calculates
 * the appropriate register values for current modulation setting.
 *
 * @param tint_us Integration time in microseconds (1 to ~6500us typical).
 * @return EPC_OK on success, EPC_ERR_PARAM if out of range.
 */
epc_status_t epc660_cfg_set_integration_time_us(uint16_t tint_us);

/**
 * @brief Set integration time via raw register values.
 *
 * Writes registers 0xA0:0xA1 (multiplier) and 0xA2:0xA3 (base count).
 *
 * @param multiplier 16-bit value for 0xA0:A1.
 * @param base_count 16-bit value for 0xA2:A3.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_integration_time_raw(uint16_t multiplier, uint16_t base_count);

/**
 * @brief Set secondary integration time for dual-integration (HDR) mode.
 *
 * Writes registers 0x9E:0x9F for the odd-row integration time.
 * Only effective when dual integration mode is enabled.
 *
 * @param base_count 16-bit value for 0x9E:9F.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_integration_time2_raw(uint16_t base_count);

/*
 * -----------------------------------------------------------------------------
 * MEASUREMENT MODE CONFIGURATION
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Set measurement mode (4-DCS, 2-DCS, grayscale, dual modes).
 *
 * Writes register 0x92 with the selected mode.
 *
 * @param mode One of epc_measurement_mode_t values.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_measurement_mode(epc_measurement_mode_t mode);

/**
 * @brief Configure grayscale sub-mode (LED on/off, modulated/DC).
 *
 * Writes register 0x3C. Only effective when in grayscale mode.
 *
 * @param gs_mode One of epc_grayscale_mode_t values.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_grayscale_mode(epc_grayscale_mode_t gs_mode);

/**
 * @brief Enable or disable continuous (auto-run) acquisition.
 *
 * When enabled (bit 1 of 0xA4), chip continuously acquires frames.
 * When disabled, each SHUTTER triggers one measurement cycle only.
 *
 * @param enable 1 to enable auto-run, 0 for single-shot.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_auto_run(uint8_t enable);

/**
 * @brief Enable dual-phase mode for motion blur reduction.
 *
 * Acquires 2 phase-shifted DCS simultaneously (DCS0+DCS1 or DCS2+DCS3).
 * Effective resolution becomes 320x120 but eliminates motion blur.
 *
 * Sets register 0x94 bit 7, plus modulation table registers 0x22, 0x25.
 *
 * @param enable 1 to enable, 0 to disable (restore defaults).
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_dual_phase_mode(uint8_t enable);

/**
 * @brief Enable dual-integration (HDR) mode.
 *
 * Even/odd rows integrate with different times for high dynamic range.
 * Effective resolution becomes 320x120 with 2 exposure levels.
 *
 * Sets register 0x94 bit 7.
 *
 * @param enable 1 to enable, 0 to disable.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_dual_integration_mode(uint8_t enable);

/*
 * -----------------------------------------------------------------------------
 * ROI, BINNING, AND RESOLUTION CONFIGURATION
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Configure Region Of Interest (ROI).
 *
 * Sets the rectangular sub-region to read out. Reduces frame time
 * proportionally to the number of rows. ROI is mirrored for top/bottom
 * pixel fields automatically.
 *
 * Writes registers 0x96-0x9B.
 *
 * Constraints:
 *   - Minimum ROI: 6 columns x 2 rows.
 *   - x_start must be >= 4 (column 4), x_end <= 323.
 *   - y_start must be >= 6 (row 6), y_end <= 125 (top half).
 *   - Start coordinates should be even, end coordinates odd.
 *
 * @param x_start Start column (4-322, even).
 * @param y_start Start row (6-124, even).
 * @param x_end End column (5-323, odd).
 * @param y_end End row (7-125, odd).
 * @return EPC_OK on success, EPC_ERR_PARAM if invalid.
 */
epc_status_t epc660_cfg_set_roi(uint16_t x_start, uint16_t y_start,
                                 uint16_t x_end, uint16_t y_end);

/**
 * @brief Reset ROI to full frame (320x240).
 *
 * Sets ROI to default: top-left (4,6), bottom-right (323,125).
 *
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_reset_roi(void);

/**
 * @brief Configure pixel binning mode.
 *
 * Combines adjacent pixels for higher sensitivity at reduced resolution.
 * Cannot be used with dual-phase or dual-integration modes.
 *
 * Writes register 0x94 bits 0-1.
 *
 * @param binning One of epc_binning_mode_t values.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_binning(epc_binning_mode_t binning);

/**
 * @brief Configure row reduction (read every Nth row).
 *
 * Increases frame rate by skipping rows.
 *
 * Writes register 0x94 bits 2-3.
 *
 * @param reduction One of epc_row_reduction_t values.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_row_reduction(epc_row_reduction_t reduction);

/**
 * @brief Configure column reduction (read every 2nd column).
 *
 * Writes register 0x94 bit 4.
 *
 * @param reduction One of epc_col_reduction_t values.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_col_reduction(epc_col_reduction_t reduction);

/*
 * -----------------------------------------------------------------------------
 * LED/ILLUMINATION DRIVER CONFIGURATION
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Select LED driver output pin (LED or LED2).
 *
 * LED: Open-drain, up to 200mA sink @ Tj<85C.
 * LED2: Push-pull, up to 50mA, better rise/fall symmetry.
 *
 * Do NOT use both simultaneously.
 *
 * Writes register 0x90.
 *
 * @param output EPC_LED_OPEN_DRAIN or EPC_LED_PUSH_PULL.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_led_output(epc_led_output_t output);

/**
 * @brief Configure LED driver polarity.
 *
 * Sets the active polarity of the LED/LED2 output.
 *
 * Writes register 0x90 bit 0.
 *
 * @param active_high 1 for active-high, 0 for active-low.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_led_polarity(uint8_t active_high);

/**
 * @brief Enable LED preheat for improved ambient-light suppression.
 *
 * Per datasheet section 1.6, enables ~33us LED modulation before
 * integration for 20% better ALS performance.
 *
 * Sets register 0x90 bit 3 and register 0xAB = 0x01.
 *
 * @param enable 1 to enable, 0 to disable.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_led_preheat(uint8_t enable);

/*
 * -----------------------------------------------------------------------------
 * DLL (DELAY LINE) CONFIGURATION
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Enable manual DLL control.
 *
 * When enabled, the LED modulation can be phase-shifted relative to
 * demodulation to optimize accuracy at specific distance ranges.
 *
 * Writes register 0xAE = 0x04.
 *
 * @param enable 1 to enable manual DLL, 0 to disable.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_dll_enable(uint8_t enable);

/**
 * @brief Set DLL coarse delay steps.
 *
 * Each step ~2.1ns, corresponding to ~30cm distance shift.
 * Range: 0-49 steps.
 *
 * Writes register 0x73.
 *
 * @param steps Number of DLL delay steps (0-49).
 * @return EPC_OK on success, EPC_ERR_PARAM if >49.
 */
epc_status_t epc660_cfg_set_dll_coarse(uint8_t steps);

/**
 * @brief Set DLL fine delay (registers 0x71/0x72).
 *
 * For precise phase tuning.
 *
 * @param fine_value 16-bit fine delay value.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_dll_fine(uint16_t fine_value);

/*
 * -----------------------------------------------------------------------------
 * TCMI (PARALLEL DATA OUTPUT) CONFIGURATION
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Set TCMI data output format.
 *
 * Writes register 0xCB.
 *
 * @param format One of epc_tcmi_format_t values.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_tcmi_format(epc_tcmi_format_t format);

/**
 * @brief Set TCMI DCLK frequency.
 *
 * Writes register 0x89.
 *
 * @param freq One of epc_dclk_freq_t values.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_dclk_freq(epc_dclk_freq_t freq);

/**
 * @brief Configure TCMI signal polarities.
 *
 * Sets active levels for VSYNC, HSYNC, DCLK, and saturation flag.
 *
 * Writes register 0xCC.
 *
 * @param vsync_active_high VSYNC active level (1=high, 0=low).
 * @param hsync_active_high HSYNC active level.
 * @param dclk_active_high DCLK active edge.
 * @param sat_on_xsync 1 to output saturation on XSYNC_SAT pin.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_tcmi_polarity(uint8_t vsync_active_high,
                                           uint8_t hsync_active_high,
                                           uint8_t dclk_active_high,
                                           uint8_t sat_on_xsync);

/**
 * @brief Enable HSYNC stretching for slower DCLK rates.
 *
 * Required when DCLK < 13MHz to avoid readout rollover.
 * Adds 2us per ADC conversion.
 *
 * Writes register 0x91 bit 6.
 *
 * @param enable 1 to enable stretching, 0 to disable.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_hsync_stretch(uint8_t enable);

/*
 * -----------------------------------------------------------------------------
 * EXTERNAL MODULATION CLOCK CONFIGURATION
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Enable external modulation clock (MODCLK input).
 *
 * When enabled, external MODCLK (up to 96MHz) is used instead of
 * internal PLL-generated clock. Useful for multi-camera FDMA systems.
 *
 * Writes register 0x80 bit 6.
 *
 * @param enable 1 to use external MODCLK, 0 for internal.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_external_modclk(uint8_t enable);

/*
 * -----------------------------------------------------------------------------
 * TEMPERATURE SENSOR CONFIGURATION
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Initialize temperature sensor readout.
 *
 * Saves and modifies trim registers (0xD3, 0xD5, 0xDA, 0xDC) and
 * reads factory calibration values from EEPROM (0xE8-0xEE).
 *
 * Must be called once after power-up before reading temperature.
 *
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_temp_sensor_init(void);

/**
 * @brief Read all four temperature sensors (raw values).
 *
 * Acquires a grayscale image in temperature sensing mode and reads
 * registers 0x60-0x67. Must call epc660_cfg_temp_sensor_init() first.
 *
 * Sensitivity in this mode is 2.5x lower than regular grayscale.
 *
 * @param raw_temps Array of 4 uint16_t to receive raw sensor values.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_temp_read_raw(uint16_t raw_temps[4]);

/**
 * @brief Convert raw temperature to degrees Celsius.
 *
 * Formula: Temp = (raw - 0x2000) * 0.134 + Z
 * where Z is the normalized calibration offset.
 *
 * @param raw_value Raw 16-bit sensor reading.
 * @param cal_offset Calibration offset (from init).
 * @return Temperature in degrees Celsius (as float, cast if needed).
 */
float epc660_cfg_temp_to_celsius(uint16_t raw_value, int16_t cal_offset);

/**
 * @brief Restore trim registers after temperature reading.
 *
 * Restores registers 0xD3, 0xD5, 0xDA, 0xDC to original values
 * for normal imaging operation.
 *
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_temp_sensor_restore(void);

/*
 * -----------------------------------------------------------------------------
 * SATURATION THRESHOLD CONFIGURATION
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Set software saturation threshold.
 *
 * Adjusts the threshold for the saturation flag. Lower values make
 * detection more sensitive.
 *
 * Writes register 0xAF.
 *
 * @param threshold Saturation threshold value (default 0x39 for 125us tint).
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_saturation_threshold(uint8_t threshold);

/**
 * @brief Configure saturation output behavior.
 *
 * Options:
 *   - Output saturation flag on XSYNC_SAT pin (default).
 *   - Force DATA[11:0] = 0xFFF when saturated.
 *
 * Writes register 0xCC bits 6-7.
 *
 * @param force_fff 1 to force 0xFFF on saturation, 0 for normal.
 * @param use_xsync 1 to output sat flag on XSYNC_SAT pin.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_saturation_output(uint8_t force_fff, uint8_t use_xsync);

/*
 * -----------------------------------------------------------------------------
 * ROLLING DCS MODE CONFIGURATION
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Enable enhanced rolling DCS frame mode.
 *
 * Achieves up to 158 fps by interleaving integration and readout.
 * Per datasheet section 11.6 and 11.7.
 *
 * @param enable 1 to enable rolling mode, 0 for standard.
 * @return EPC_OK on success.
 */
epc_status_t epc660_cfg_set_rolling_mode(uint8_t enable);

/* ===== AI-GENERATED BLOCK END ===== */

#endif /* INC_EPC660_CONFIG_H_ */
