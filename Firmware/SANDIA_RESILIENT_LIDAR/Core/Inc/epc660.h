/*
 * epc660.h
 *
 *  Created on: Jan 22, 2026
 *      Author: unrea
 */

#ifndef INC_EPC660_H_
#define INC_EPC660_H_

#include "epc660_platform.h"

// TYPE DEFINITIONS

// FUNCTION PROTOTYPES
epc_status_t epc660_power_up(void);
void epc660_power_down(void);
void epc_emergency_power_down(void);

/* ===== AI-GENERATED BLOCK START ===== */
/*
 * -----------------------------------------------------------------------------
 * HIGH-LEVEL RUNTIME & LIFECYCLE FUNCTIONS
 * These functions control the overall chip lifecycle, initialization, and
 * runtime operations. For low-level configuration (modes, timing, ROI, etc.),
 * see epc660_config.h.
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Initialize the EPC660 after power-up.
 *
 * Expected to:
 * - Verify chip ID (0x01) and EEPROM->CFG ready status (0x7E bit 0).
 * - Load pixel sequencer code to RAM.
 * - Apply default configuration registers per datasheet Table 22.
 * - Enable LED preheat (0x90 bit 3, 0xAB = 0x01).
 * - Enable manual DLL control (0xAE = 0x04).
 *
 * @return EPC_OK on success, EPC_ERR_ID if chip ID mismatch, EPC_ERR otherwise.
 */
epc_status_t epc660_init(void);

/**
 * @brief Perform a soft reset of the EPC660 via I2C general call.
 *
 * Sends the I2C general call software reset command (0x00, 0x06).
 * Chip will reload EEPROM->CFG and require re-initialization.
 *
 * @return EPC_OK on success.
 */
epc_status_t epc660_soft_reset(void);

// Seems like a useless abstraction
///**
// * @brief Load the EPC660 pixel sequencer program (RAM).
// *
// * Wrapper that calls epc660_sequencer_load() from epc660_sequencer.h.
// * Must be called during init before any measurement.
// *
// * @return EPC_OK on success.
// */
//epc_status_t epc660_load_sequencer(void);

/**
 * @brief Trigger a single measurement using SW shutter (register 0xA4 bit 0).
 *
 * Initiates one measurement cycle (1/2/4 DCS frames per current mode).
 * Data is output via TCMI after integration + conversion.
 *
 * @return EPC_OK on success.
 */
epc_status_t epc660_trigger_single_measurement(void);

/**
 * @brief Trigger a single measurement using HW SHUTTER pin.
 *
 * Pulses the SHUTTER GPIO for >= 250ns to trigger measurement.
 * Faster than SW shutter by ~26us.
 *
 * @return EPC_OK on success.
 */
void epc660_trigger_hw_shutter(void);

/**
 * @brief Read current status/ready flags from the device.
 *
 * Reads register 0x7E:
 *   - Bit 0: EEPROM->CFG copy complete.
 *
 * @param status_out Pointer to receive status byte.
 * @return EPC_OK on success.
 */
epc_status_t epc660_get_status(uint8_t *status_out);

/**
 * @brief Read chip ID (IC type) and IC version.
 *
 * IC type at register 0x01, part version at register 0xFB.
 *
 * @param chip_id_out Pointer to receive chip ID byte.
 * @param ic_version_out Pointer to receive IC version byte.
 * @return EPC_OK on success.
 */
epc_status_t epc660_get_id(uint8_t *chip_id_out, uint8_t *ic_version_out);

/**
 * @brief Read wafer ID and chip ID from EEPROM for unique identification.
 *
 * Wafer ID at EEPROM 0xFC-0xFD, Chip ID at 0xFE-0xFF.
 *
 * @param wafer_id_out Pointer to receive 16-bit wafer ID.
 * @param chip_id_out Pointer to receive 16-bit chip ID.
 * @return EPC_OK on success.
 */
epc_status_t epc660_get_unique_id(uint16_t *wafer_id_out, uint16_t *chip_id_out);

/**
 * @brief Read a DCS or grayscale frame via TCMI/parallel capture (DCMI/DMA).
 *
 * This is a placeholder for the MCU-side parallel capture implementation.
 * Actual capture depends on your DCMI/GPIO+DMA setup.
 *
 * @param buffer Destination buffer for pixel data (12-bit per pixel).
 * @param buffer_len Bytes available in buffer.
 * @param bytes_written Pointer to receive actual bytes written.
 * @return EPC_OK on success, EPC_ERR_PARAM if buffer too small.
 */
epc_status_t epc660_capture_frame(uint16_t *buffer, uint32_t buffer_len, uint32_t *bytes_written);

/**
 * @brief Read a register (direct address space 0x00-0x7F, RAM 0x80-0xEF).
 *
 * @param reg_addr Register address.
 * @param value_out Pointer to receive value.
 * @return EPC_OK on success.
 */
epc_status_t epc660_read_reg(uint8_t reg_addr, uint8_t *value_out);

/**
 * @brief Write a register (direct address space).
 *
 * @param reg_addr Register address.
 * @param value Value to write.
 * @return EPC_OK on success.
 */
epc_status_t epc660_write_reg(uint8_t reg_addr, uint8_t value);

/**
 * @brief Read an EEPROM byte via indirect access (0xF0-0xFF address space).
 *
 * Uses pointer register 0x11 and data register 0x12.
 *
 * @param eeprom_addr EEPROM address (0xF0-0xFF range).
 * @param value_out Pointer to receive value.
 * @return EPC_OK on success.
 */
epc_status_t epc660_read_eeprom(uint8_t eeprom_addr, uint8_t *value_out);

/**
 * @brief Write an EEPROM byte via indirect access.
 *
 * Uses pointer register 0x11 and data register 0x12.
 * Takes ~25ms per byte write.
 *
 * @param eeprom_addr EEPROM address.
 * @param value Value to write.
 * @return EPC_OK on success.
 */
epc_status_t epc660_write_eeprom(uint8_t eeprom_addr, uint8_t value);

/**
 * @brief Load calibration parameters from EEPROM into runtime registers.
 *
 * Triggered automatically after reset; can be called manually if needed.
 *
 * @return EPC_OK on success.
 */
epc_status_t epc660_load_calibration(void);
/* ===== AI-GENERATED BLOCK END ===== */


#endif /* INC_EPC660_H_ */
