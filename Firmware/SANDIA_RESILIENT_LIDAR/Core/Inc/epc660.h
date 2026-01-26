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


#endif /* INC_EPC660_H_ */
