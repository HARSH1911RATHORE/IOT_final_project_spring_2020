

/*****************************************************************************
*HARSH RATHORE
*UNIVERSITY OF COLORADO BOULDER
*SPRING 2020
*IOT EMBEDDED FIRMWARE
******************************************************************************/
///// **@file name Server.h
///// *@Server bluetooth func
///// *this header file contains all the bluetooth stack event for Server
///// *@reference SI LABS API, SOC THERMOMETER
///// **This header file provides the Server functions
///// *@author Harsh Rathore
///// @ date 16TH FEB
/////@versionâ1.0

#ifndef SRC_Server_H_
#define SRC_Server_H_

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
#include "state_machine.h"
#include "main.h"
#include "infrastructure.h"
#include "gatt_db.h"
#include "display.h"
#include "math.h"
#include "ble_device_type.h"

/**
 * bluetooth stack function to handle the specific commands and events
 *
 * @param evt which will be passed from gecko external signal
 * @return void
 */
void gecko_ecen5823_update(struct gecko_cmd_packet* evt);

#endif
