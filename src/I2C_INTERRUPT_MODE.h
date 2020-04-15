
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
#endif
