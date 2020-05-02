/*****************************************************************************
*HARSH RATHORE
*UNIVERSITY OF COLORADO BOULDER
*SPRING 2020
*IOT EMBEDDED FIRMWARE
******************************************************************************/
///// **@file name main.h
///// *@app main
///// *@reference https://www.silabs.com/documents/login/reference-manuals/bluetooth-le-and-mesh-software-api-reference-manual.pd
///// **This header file provides the app main
///// *@author Harsh Rathore
///// @ date 16TH FEB
/////@versionâ1.0
///// /

#ifndef _MAIN_H_
#define _MAIN_H_

#define connection_int_max (60)
#define connection_int_min (60)
#define slave_latency (3)
#define timeout (500)
#define advertsing_interval_min (400)
#define advertsing_interval_max (400)
#define duration (0)
#define maxevents (0)
#define handle_1 (0)
#define characteristic_handle (5)
#define connection_all (0xFF)

/**
 * @brief - temperature conversion to a format which can be seen properly on bluetooth app
 *
 * @param void
 * @return void
 */

void temperatureMeasure();

//~~
void humidityMeasure();
//~~

/**
 * @brief - shutdown i2c and letime
 *
 * @param void
 * @return void
 */

void shut_down_i2c_letimer();
/**
 * bluetooth stack function to handle the specific commands and events
 *
 * @param evt which will be passed from gecko external signal
 *
 */
void gecko_ecen5823_update(struct gecko_cmd_packet* evt);

/**
 * app main reference
 *
 * @param configuration of gecko
 *
 */
int appMain(gecko_configuration_t *config);

#endif
