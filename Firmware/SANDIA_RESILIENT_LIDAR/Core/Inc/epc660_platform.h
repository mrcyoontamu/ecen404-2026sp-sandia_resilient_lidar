/*
 * epc660_platform.h
 *
 *  Created on: Jan 22, 2026
 *      Author: unrea
 */

#ifndef INC_EPC660_PLATFORM_H_
#define INC_EPC660_PLATFORM_H_

#include <stdint.h>

// CUSTOM TYPE DEFINITIONS
typedef enum {
    EPC_OK              = 0,    // Operation successful
    EPC_ERR           	= -1,   // Generic error
    EPC_ERR_TIMEOUT     = -3,   // Wait function timed out
	EPC_ERR_BUSY		= -4,
    EPC_ERR_ID          = -5,   // Chip ID mismatch (Hardware connection issue)
    EPC_ERR_PARAM       = -6    // Invalid parameter passed to function
} epc_status_t;

typedef enum {
	EPC_DIRECT   = 0,  // Standard I2C write
	EPC_INDIRECT = 1   // Special EEPROM/Sequencer write
} epc_i2c_access_t;

// FUNCTION PROTOTYPES
void epc_reset_pin_set(int state);

void epc_power_1v8_3v3_set(int state);
void epc_power_5v_10v_set(int state);
void epc_power_neg10v_set(int state);
void epc_power_15v_set(int state);

void epc_delay_ms(uint32_t ms);

epc_status_t epc_i2c_write(uint8_t reg_addr, uint8_t value, epc_i2c_access_t type);
epc_status_t epc_i2c_read(uint8_t reg_addr, uint8_t *value, epc_i2c_access_t type);
epc_status_t epc_i2c_write_multi(uint8_t reg_addr, uint8_t *data, uint16_t len);
epc_status_t epc_i2c_read_multi(uint8_t reg_addr, uint8_t *data, uint16_t len);

#endif /* INC_EPC660_PLATFORM_H_ */
