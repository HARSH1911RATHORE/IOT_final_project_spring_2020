/*****************************************************************************
*HARSH RATHORE
*UNIVERSITY OF COLORADO BOULDER
*SPRING 2020
*IOT EMBEDDED FIRMWARE
******************************************************************************/
///// **@file name state_machine.h
///// *@state_machine i2c
///// *this header file contains all the variables, states and events for state machine of i2c
///// *@reference si7021 datasheet-https://www.silabs.com/documents/public/data-sheets/Si7021-A20.pdf // https://www.silabs.com/documents/login/reference-manuals/bluetooth-le-and-mesh-software-api-reference-manual.pd
///// **This header file provides the state machine functions
///// *@author Harsh Rathore
///// @ date 16TH FEB
/////@versionâ1.0



#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_
#include "gecko_configuration.h"
#include "gpio.h"
#include "native_gecko.h"             //enabling all required libraries for low energy timer,i2c, logging and sleep routines
#include "em_cmu.h"
#include "em_emu.h"
#include "em_letimer.h"
#include "em_core.h"
#include "sleep.h"
#include <stdio.h>
#include "letimer.h"
#include "log.h"
#include "em_i2c.h"
#include "i2c.h"
#include "I2C_INTERRUPT_MODE.h"

//~~~~~~~~~~~~~~~~~~~~~~~~

#define humidity_read 100
#define aqi_read   50
#define Air_quality_index_key (0x4001)
#define Humidity_key (0x4070)
//~~~~~~~~~~~~~~~~~~~~~~~~

typedef enum
{
	power_up,                                       //states of i2c transaction
	wait_for_i2c_write_start,
	wait_for_i2c_read_start,
	display,
	sleeping
}i2c_temp_states;
typedef enum
{ 													//events
	setup_timer_event,
	start_i2c_write,
	setup_timer_event1,
	start_i2c_read,
	turn_power_off
}i2c_events;

i2c_temp_states current_state;                //current i2c state
i2c_events events;

extern  uint32_t event_bluetooth;
extern  uint32_t event_timer_10_bluetooth;								//event processing bits to be passed in the gecko_external_signal function to be processed after bluetooth is initialized
extern   uint32_t event_timer_80_bluetooth;
extern  uint32_t event_timer_5_bluetooth;
extern  uint32_t event_read_bluetooth;
extern  uint32_t event_write_bluetooth;
extern  uint32_t event_i2c_progress_write;
extern  uint32_t event_i2c_progress_read;

extern bool event_i2c_progressing_write;								//write in progress event
extern bool event_i2c_progressing_read;									//read in progress event

#define timer_5  1														//timer 5 mili second event
#define timer_10 2														//timer 10 mili second event
#define timer_80 3                                                 		//timer 80 mili second event
#define read_state 4													//read state event
#define write_state 5													//write state event

extern int conn_open_id;												//bluetooth connection open
extern bool lcd;
/**
 * @brief - switches state as events get completed and helps in achieving i2c interrupt
 *
 * @param none
 * @return float value of temperature
 */
//~~~
//~~~~~~~~~~~~~~
typedef enum
{
	configure,                                       //states of i2c transaction
	read_aqi,
}i2c_aqi_states;
typedef enum
{ 													//events
	setup_timer_event_aqi,
	start_i2c_write__aqi,
	setup_timer_event1_aqi,
	start_i2c_read_aqi,
	turn_power_off_aqi
}i2c_aqi_events;


i2c_aqi_states current_state_aqi;                //current i2c state
i2c_aqi_events events_aqi;
//~~~~~~~~~~~~~
extern bool event_configure_aqi;
extern uint16_t event_aqi_wr_done;
extern uint16_t event_aqi_wr_progress;

void ppm_poll();
void humid_get();
void aqi_sensor_init();
void state_machine_i2c_humidity();
void state_machine_i2c_aqi();
//~~~~~


#endif
