
/*****************************************************************************
*HARSH RATHORE
*UNIVERSITY OF COLORADO BOULDER
*SPRING 2020
*IOT EMBEDDED FIRMWARE
******************************************************************************/
///// **@file name I2C_INTERRUPT_MODE.h
///// *@i2c interrupt functions
///// *this header file contains all the i2c interrupt functions, variables and flags
///// *@reference SI LABS API
///// @reference - https://cdn.sparkfun.com/datasheets/BreakoutBoards/CCS811_Programming_Guide.pdf
///// @reference - https://github.com/CU-ECEN-5823/course-project-PuneetBansal
///// @reference - SI LABS API
///// **This header file provides the app main
///// *@author Harsh Rathore
///// @ date 16TH FEB
/////@versionâ1.0



#ifndef _I2C_INT_MODE_
#define _I2C_INT_MODE_
#include "em_i2c.h"
#include "em_gpio.h"
#include "i2cspm.h"                            //declaring all libraries needed by i2c functions
#include "log.h"
#include "letimer.h"
#include "em_core.h"
#include "sleep.h"
#include "gpio.h"


//int bluetooth_i2c=0;

extern bool event_write;
extern bool event_read;                                                     //event flags for write and read

/**
 * @brief - initiates the transfer init function to start with i2c interrupt
 *
 * @param none
 * @return void
 */

void I2C_transfer_interrupt_init();
/**
 * @brief - initialises the read i2c structure and transfer init for read i2c interrupt
 *
 * @param none
 * @return void
 */

void I2C_read_interrupt();
/**
 * @brief - reads the temperature value after it is available in the buffer every 3 second period
 *
 * @param none
 * @return float value of temperature
 */

float I2C_Read_temp_value();
/**
 * @brief - i2c irq handler which checks if transfer is done in case of read and write and sets the flags
 *
 * @param void
 * @return void
 */


void I2C0_IRQHandler(void);

/**
 * @brief - i2c based write write functions
 *
 * @param void
 * @return void
 */
void WriteWrite_int_i2c();

/**
 * @brief - i2c read humidity calculation function
 *
 * @param void
 * @return void
 */
float I2C_Read_humidity_value();

/**
 * @brief - function to write and read to i2c based device
 *
 * @param length of write and read
 * @return void
 */


void WRITE_READ_AQI(uint8_t len_write,uint8_t len_read );

#endif
