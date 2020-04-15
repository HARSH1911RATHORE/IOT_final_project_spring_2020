
/*****************************************************************************
*HARSH RATHORE
*UNIVERSITY OF COLORADO BOULDER
*SPRING 2020
*IOT EMBEDDED FIRMWARE
******************************************************************************/
///// **@file name letimer.h
///// *@letimer clock, initilaisation and timer
///// *this header file contains all the letimer interrupt functions, variables and flags and timerwaitus and timereventms functions
///// *@reference SI LABS API
///// **This header file provides the letimer functions
///// *@author Harsh Rathore
///// @ date 16TH FEB
/////@versionâ1.0


#ifndef _letimer_h_
#define _letimer_h_

#include "em_cmu.h"
#include "em_letimer.h"
#include "sleep.h"                 //defining library functions required by letiemr
#include "gpio.h"
#include "em_gpio.h"
#include "i2c.h"
#include "em_core.h"
#include "log.h"


#define total_period 3             //setting the led on and off total time


#define count_value_max 65536                                           //total count value of 16 bit register

#define led_off (total_period-led_on)                                   //led off period is total period minus led on

#define max_time_frequency (0.125)                                     //time taken on count of the entire letimer register for low frequency clock
#define max_time_low_clock_frequency (1/65.536)                        //time taken on count of the entire letimer register for ultra low frequency clock
#define period_set (max_time_frequency*count_value_max*total_period)   //count value to be loaded in the letimer comp0 register for lfxo

#define period_set_deep_sleep (max_time_low_clock_frequency*count_value_max*total_period)  //count value to be loaded in the letimer comp0 register for ulfxo

float period;                                                                             //period to be loaded in count register COMP0
#define led_on_counter_low  (max_time_frequency*count_value*led_on)                       //count value to be loaded in the letimer comp1 register for lfxo
#define led_on_counter_u_low  (max_time_low_clock_frequency*count_value*led_on)           //count value to be loaded in the letimer comp1 register for lfxo
float led_on_counter;



extern bool event_timer_80;                    //80 ms timereventms
extern bool event_timer_10;                     //flags set when the event for the particular state is triggered by an interrupt
extern bool event_timer_5;

extern bool event;								//event uf interrupt


extern int conn_open_id;

/**
 * @brief - sets the clock for timer depending on the mode the system is operating in
 *
 * @param void
 * @return void
 */

void clockInit();
/**
 * @brief - letimer initializationng in
 *
 * @param void
 * @return void
 */

void letimerInit();
/**
 * @brief - sets the letimer irq handler which triggers depending on the compare value
 *
 * @param void
 * @return void
 */

void LETIMER0_IRQHandler(void);
/**
 * @brief - sets the timer delay which will be given using letimer registers through polling
 *
 * @param time the wait period will be in micro seconds
 * @return void
 */

void timerWaitUs(int us_wait);
/**
 * @brief - sets the timer delay which will be given using letimer interrupt
 *
 * @param ms unitl wait
 * @return void
 */

void timerSetEventInMs(int ms_until_wait);
#endif
