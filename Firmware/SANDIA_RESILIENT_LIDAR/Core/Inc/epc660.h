/*
 * epc660.h
 *
 *  Created on: Jan 22, 2026
 *      Author: unrea
 */

#ifndef INC_EPC660_H_
#define INC_EPC660_H_

typedef enum {
    EPC_DIRECT   = 0,  // Standard I2C write
    EPC_INDIRECT = 1   // Special EEPROM/Sequencer write
} epc_access_t;

#endif /* INC_EPC660_H_ */
