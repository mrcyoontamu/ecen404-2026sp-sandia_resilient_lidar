/*
 * epc660_reg.h
 *
 *  Created on: Jan 22, 2026
 *      Author: unrea
 */

#ifndef INC_EPC660_REG_H_
#define INC_EPC660_REG_H_


#define EPC_I2C_ADDR					(0x20 << 1)

// ID Registers
#define EPC_REG_IC_TYPE					0x00
#define EPC_REG_IC_VERSION				0x01
#define EPC_CHIP_ID_EXPECTED			0x02

#define EPC_REG_GRAYSCALE_MODE_CTRL		0x3C
#define EPC_REG_SHUTTER_CTRL			0xA4

// Integration time
#define EPC_REG_INT_MULT_LOW			0xA0
#define EPC_REG_INT_MULT_HIGH			0xA1
#define EPC_REG_INT_BASE_LOW			0xA2	// Integration time low bit
#define EPC_REG_INT_BASE_HIGH			0xA3	// Integration time high bit

// EEPROM stuff
#define EPC_REG_RAM_PAGE				0x80
#define EPC_REG_EEPROM_ADDR				0x11
#define EPC_REG_EEPROM_DATA				0x12

// On Boot-up
#define EPC_REG_CFG_READY				0x7E

// Temperature Sensors
#define EPC_REG_TEMP_TL					0x60	// Top-left sensor
#define EPC_REG_TEMP_TR					0x62	// Top-right sensor
#define EPC_REG_TEMP_BL					0x64
#define EPC_REG_TEMP_BR					0x66

// Clock timing
#define EPC_REG_MOD_DIV					0x85
#define EPC_REG_DCLK_CTRL				0x89

// Measurement mode
#define EPC_REG_MEASUREMENT_MODE		0x92



//  AI GENERATED FOR REFERENCE
// /* Default 7-bit I2C device address (A1=A0=0). */
// #define EPC660_I2C_ADDR_DEFAULT                 0x20U

// /* Control page (0x00 - 0x7F) */
// #define EPC660_REG_IC_TYPE                      0x00U
// #define EPC660_REG_IC_VERSION                   0x01U
// #define EPC660_REG_EE_ADDR                      0x11U
// #define EPC660_REG_EE_DATA                      0x12U
// #define EPC660_REG_STRAP_SCAN                   0x20U
// #define EPC660_REG_DCS_ABS_1ST                  0x22U
// #define EPC660_REG_MOD_CTRL_1ST                 0x24U
// #define EPC660_REG_DCS_ABS_2ND                  0x25U
// #define EPC660_REG_MOD_CTRL_2ND                 0x27U
// #define EPC660_REG_DCS_ABS_3RD                  0x28U
// #define EPC660_REG_MOD_CTRL_3RD                 0x2AU
// #define EPC660_REG_DCS_ABS_4TH                  0x2BU
// #define EPC660_REG_MOD_CTRL_4TH                 0x2DU
// #define EPC660_REG_GRAYSCALE_READOUT_MODE       0x3AU
// #define EPC660_REG_GRAYSCALE_MOD_CTRL           0x3CU
// #define EPC660_REG_TEMP_TL_SUM                  0x60U
// #define EPC660_REG_TEMP_TL_SUM_MSB              0x61U
// #define EPC660_REG_TEMP_TR_SUM                  0x62U
// #define EPC660_REG_TEMP_TR_SUM_MSB              0x63U
// #define EPC660_REG_TEMP_BL_SUM                  0x64U
// #define EPC660_REG_TEMP_BL_SUM_MSB              0x65U
// #define EPC660_REG_TEMP_BR_SUM                  0x66U
// #define EPC660_REG_TEMP_BR_SUM_MSB              0x67U
// #define EPC660_REG_DLL_FINE                     0x71U
// #define EPC660_REG_DLL_FINE_MSB                 0x72U
// #define EPC660_REG_DLL_COARSE                   0x73U
// #define EPC660_REG_MODE_CTRL                    0x7DU

// /* RAM page (0x80 - 0xEF) */
// #define EPC660_REG_CLOCK_CTRL                   0x80U
// #define EPC660_REG_MODCLK_DIV                   0x85U
// #define EPC660_REG_TCMI_CLK_CTRL                0x89U
// #define EPC660_REG_DEMOD_DELAY                  0x8BU
// #define EPC660_REG_LED_CTRL                     0x90U
// #define EPC660_REG_SEQ_CTRL                     0x91U
// #define EPC660_REG_MOD_SELECT                   0x92U
// #define EPC660_REG_RES_BIN_MODE                 0x94U
// #define EPC660_REG_ROI_TL_X                     0x96U
// #define EPC660_REG_ROI_TL_X_MSB                 0x97U
// #define EPC660_REG_ROI_BR_X                     0x98U
// #define EPC660_REG_ROI_BR_X_MSB                 0x99U
// #define EPC660_REG_ROI_TL_Y                     0x9AU
// #define EPC660_REG_ROI_BR_Y                     0x9BU
// #define EPC660_REG_INT_LEN2                     0x9EU
// #define EPC660_REG_INT_LEN2_MSB                 0x9FU
// #define EPC660_REG_INT_MUL                      0xA0U
// #define EPC660_REG_INT_MUL_MSB                  0xA1U
// #define EPC660_REG_INT_LEN1                     0xA2U
// #define EPC660_REG_INT_LEN1_MSB                 0xA3U
// #define EPC660_REG_SHUTTER_CTRL                 0xA4U
// #define EPC660_REG_POWER_CTRL                   0xA5U
// #define EPC660_REG_LED_PREHEAT_CFG              0xABU
// #define EPC660_REG_DLL_CTRL                     0xAEU
// #define EPC660_REG_SAT_THRESHOLD                0xAFU
// #define EPC660_REG_I2C_ADDR                     0xCAU
// #define EPC660_REG_I2C_TCMI_CTRL                0xCBU
// #define EPC660_REG_TCMI_POLARITY                0xCCU
// #define EPC660_REG_TEMP_OFFSET_TL               0xE8U
// #define EPC660_REG_DLL_STEP                     0xE9U
// #define EPC660_REG_TEMP_OFFSET_TR               0xEAU
// #define EPC660_REG_TEMP_OFFSET_BL               0xECU
// #define EPC660_REG_TEMP_OFFSET_BR               0xEEU

// /* EEPROM page (0xF0 - 0xFF) - indirect access via 0x11/0x12 */
// #define EPC660_REG_EE_USER                      0xF0U
// #define EPC660_REG_EE_CUSTOMER_ID               0xF5U
// #define EPC660_REG_EE_WAFER_ID_MSB              0xF6U
// #define EPC660_REG_EE_WAFER_ID_LSB              0xF7U
// #define EPC660_REG_EE_CHIP_ID_MSB               0xF8U
// #define EPC660_REG_EE_CHIP_ID_LSB               0xF9U
// #define EPC660_REG_EE_PART_TYPE                 0xFAU
// #define EPC660_REG_EE_PART_VERSION              0xFBU

// /* Bit masks: STRAP scan (0x20) */
// #define EPC660_STRAP_A0_MASK                    (1U << 5)
// #define EPC660_STRAP_A1_MASK                    (1U << 6)

// /* Bit masks: DCS/ABS selection (0x22/0x25/0x28/0x2B) */
// #define EPC660_DCS_MGX0_MASK                    (0x03U << 0)
// #define EPC660_DCS_MGX1_MASK                    (0x03U << 2)
// #define EPC660_ABS_MASK                         (0x03U << 4)

// /* Bit masks: Mode control (0x7D) */
// #define EPC660_MODE_CTRL_PLL_EN                 (1U << 2)

// /* Bit masks: Clock control (0x80) */
// #define EPC660_CLOCK_CTRL_EXT_MODCLK            (1U << 6)

// /* Bit masks: TCMI clock control (0x89) */
// #define EPC660_TCMI_CLK_DIV_MASK                (0x1FU << 0)
// #define EPC660_TCMI_CLK_SKEW_EN                 (1U << 7)

// /* Bit masks: LED driver control (0x90) */
// #define EPC660_LED_CTRL_INVERT                  (1U << 1)
// #define EPC660_LED_CTRL_LED_EN                  (1U << 2)
// #define EPC660_LED_CTRL_PREHEAT_EN              (1U << 3)
// #define EPC660_LED_CTRL_TORCH                   (1U << 4)
// #define EPC660_LED_CTRL_LED2_EN                 (1U << 5)

// /* Bit masks: Sequencer control (0x91) */
// #define EPC660_SEQ_CTRL_DCLK_STRETCH_EN         (1U << 6)

// /* Bit masks: Modulation select (0x92) */
// #define EPC660_MOD_SEL_DUAL_INT_EN              (1U << 3)
// #define EPC660_MOD_SEL_DCS_MASK                 (0x03U << 4)
// #define EPC660_MOD_SEL_MODE_MASK                (0x03U << 6)
// #define EPC660_MOD_SEL_MODE_TOF                 (0x00U << 6)
// #define EPC660_MOD_SEL_MODE_GRAYSCALE           (0x03U << 6)
// #define EPC660_MOD_SEL_DCS_1                    (0x00U << 4)
// #define EPC660_MOD_SEL_DCS_2                    (0x01U << 4)
// #define EPC660_MOD_SEL_DCS_4                    (0x03U << 4)

// /* Bit masks: Resolution/Binning/Pixel-field mode (0x94) */
// #define EPC660_RES_COL_REDUC_MASK               (0x03U << 0)
// #define EPC660_RES_ROW_REDUC_MASK               (0x03U << 2)
// #define EPC660_RES_BIN_MASK                     (0x03U << 4)
// #define EPC660_RES_PIXELFIELD_DUAL              (1U << 7)

// /* Bit masks: Shutter control (0xA4) */
// #define EPC660_SHUTTER_RELEASE                  (1U << 0)
// #define EPC660_SHUTTER_MULTI_FRAME              (1U << 1)

// /* Bit masks: Power control (0xA5) */
// #define EPC660_POWER_OFF                        0x00U
// #define EPC660_POWER_ON                         0x07U

// /* Bit masks: DLL control (0xAE) */
// #define EPC660_DLL_CTRL_NO_DELAY                0x01U
// #define EPC660_DLL_CTRL_MANUAL_DELAY            0x04U

// /* Bit masks: I2C and TCMI control (0xCB) */
// #define EPC660_I2C_STRETCH_EN                   (1U << 0)
// #define EPC660_I2C_SPIKE_FILTER_EN              (1U << 1)
// #define EPC660_TCMI_FMT_MASK                    (0x03U << 4)
// #define EPC660_TCMI_FMT_12BIT_1X                (0x00U << 4)
// #define EPC660_TCMI_FMT_8MSB_1X                 (0x01U << 4)
// #define EPC660_TCMI_FMT_LSBMSB_2X               (0x02U << 4)
// #define EPC660_TCMI_FMT_MSBLSB_2X               (0x03U << 4)
// #define EPC660_TCMI_SAT_ON_LSB_EN               (1U << 6)

// /* Bit masks: TCMI polarity (0xCC) */
// #define EPC660_TCMI_DCLK_RISING                 (1U << 0)
// #define EPC660_TCMI_HSYNC_HIGH                  (1U << 1)
// #define EPC660_TCMI_VSYNC_HIGH                  (1U << 2)
// #define EPC660_TCMI_XSYNC_HIGH                  (1U << 3)
// #define EPC660_TCMI_XSYNC_SEL_SAT               (1U << 6)
// #define EPC660_TCMI_FORCE_SAT_DATA              (1U << 7)

// /* General call commands */
// #define EPC660_GENCALL_ADDR                     0x00U
// #define EPC660_GENCALL_SOFT_RESET               0x06U
// #define EPC660_GENCALL_ADDR_RELOAD              0x04U

#endif /* INC_EPC660_REG_H_ */
