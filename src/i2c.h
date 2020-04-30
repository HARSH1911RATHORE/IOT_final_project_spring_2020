/*****************************************************************************
*HARSH RATHORE
*UNIVERSITY OF COLORADO BOULDER
*SPRING 2020
*IOT EMBEDDED FIRMWARE
******************************************************************************/
///// **@file name i2c.h
///// *@i2c functions
///// *This header file provides the functions to initialize, disable, read and write i2c and its pins
///// *@reference SI LABS API for I2C
///// **This header file provides the i2c functions
///// *@author Harsh Rathore
///// @ date 16TH FEB
/////@versionâ1.0



#ifndef _I2C_H_
#define _I2C_H_

#include "em_i2c.h"
#include "em_gpio.h"
#include "i2cspm.h"                            //declaring all libraries needed by i2c functions
#include "log.h"
#include "letimer.h"
#include "em_core.h"

#include "sleep.h"


#define slave_address 0x40                     //enabling the slave address for the temperature sensor
#define SCL_pin 10                             //scl pin defined for i2c
#define SDA_pin 11                             //sda pin defined for i2c
#define SCL_Location 14                        //scl location defined for i2c
#define SDA_Location 16                        //sda location defined for i2c
#define Reference_clock 0                      //reference clock selected 0

extern uint8_t write_data;                       //setting no hold master mode
extern uint32_t one_byte;

extern uint8_t read_data_int[2];

/**
 * @brief - initializing pins, ports for i2c
 *
 * @param void
 * @return void
 */

void init_i2c();
/**
 * @brief - sets the write structure for i2c transfer
 *
 * @param void
 * @return void
 */

void transfer_i2c();
/**
 * @brief -  sets the read structure for i2c transfer
 *
 * @param void
 * @return float to read the temperature value from read buffer
 */

float read_temp_i2c();
/**
 * @brief -  event handler which performs i2c operation every 3 seconds on being set from underflow interrupt
 *
 * @param void
 * @return void
 */

void event_handler();
/**
 * @brief -  used to disable the Gpio sensor enable pin and port for i2c for energy conservation
 *
 * @param void
 * @return void
 */

void Gpio_disable();
/**
 * @brief -  used to Enable the Gpio sensor enable pin and port for i2c for i2c transaction
 *
 * @param void
 * @return void
 */

void Gpio_enable();
/**
 * @brief -  used to Enable the Gpio sensor
 *
 * @param void
 * @return void
 */

void Gpio_sensor_enable();


//~~~~~
void init_aqi_i2c();
void HUMIDITY_POLL();
//~~~~~~
#endif
