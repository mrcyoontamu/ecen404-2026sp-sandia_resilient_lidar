/*
 * epc660_platform.h
 *
 *  Created on: Jan 22, 2026
 *      Author: unrea
 */

#ifndef INC_EPC660_PLATFORM_H_
#define INC_EPC660_PLATFORM_H_

#include <stdint.h>

void epc_reset_pin_set_low(void);
void epc_reset_pin_set_high(void);

void epc_power_1v8_3v3_on(void);
void epc_power_5v_10v_on(void);
void epc_power_neg10v_on(void);
void epc_power_15v_on(void);

void epc_delay_ms(uint32_t ms);

int epc_i2c_write(uint8_t reg_addr, uint8_t value);
int epc_i2c_read(uint8_t reg_addr, uint8_t *value);

#endif /* INC_EPC660_PLATFORM_H_ */
