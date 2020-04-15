

/*****************************************************************************
*HARSH RATHORE
*UNIVERSITY OF COLORADO BOULDER
*SPRING 2020
*IOT EMBEDDED FIRMWARE
******************************************************************************/
///// **@file name Client.h
///// *@Client bluetooth func
///// *this header file contains all the bluetooth stack event for client
///// *@reference SI LABS API,SOC THERMOMETER CLIENT
///// **This header file provides the client functions
///// *@author Harsh Rathore
///// @ date 16TH FEB
/////@versionâ1.0


#ifndef SRC_Client_H_
#define SRC_Client_H_

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


#define CONN_INTERVAL_MIN             80   //250ms
#define CONN_INTERVAL_MAX             80   //250ms
#define CONN_SLAVE_LATENCY            0    //no latency
#define CONN_TIMEOUT                  100  //500ms

#define SCAN_INTERVAL                 16   //10ms
#define SCAN_WINDOW                   16   //10ms
#define SCAN_PASSIVE                  0
#define SCAN_ACTIVE 				  1

#define TEMP_INVALID                  (uint32_t)0xFFFFFFFFu  //temp invalid
#define RSSI_INVALID                  (int8_t)127			//RSSI_INVALID
#define CONNECTION_HANDLE_INVALID     (uint8_t)0xFFu 		//CONNECTION_HANDLE_INVALID
#define SERVICE_HANDLE_INVALID        (uint32_t)0xFFFFFFFFu //SERVICE_HANDLE_INVALID
#define CHARACTERISTIC_HANDLE_INVALID (uint16_t)0xFFFFu    //CHARACTERISTIC_HANDLE_INVALID
#define TABLE_INDEX_INVALID           (uint8_t)0xFFu      //TABLE_INDEX_INVALID

#define EXT_SIGNAL_PRINT_RESULTS      0x01

// Gecko configuration parameters (see gecko_configuration.h)
#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS               20
#endif

int val;							//value of opened connection, connecion handle
uint8 button_value;					//button value presently



uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

/**
 * @bried- function to Parse advertisements looking for advertised Health Thermometer service
 *
 * @param data value, length of data value
 * @return 0 on success
 */
uint8_t findServiceInAdvertisement(uint8_t *data, uint8_t len);

/**
 * @bried- function to find index of a given connection in the connection_properties array
 * @param connection handle
 * @return TABLE_INDEX
 */
uint8_t findIndexByConnectionHandle(uint8_t connection);

/**
 * @bried- function to add connection made
 *
 * @param connection handle, address
 * @return void
 */
void addConnection(uint8_t connection, uint16_t address);

/**
 * @bried- function to remove specific connection on close id
 *
 * @param connection handle
 * @return void
 */
void removeConnection(uint8_t connection);

/**
 * bluetooth stack function to handle the specific commands and events
 *
 * @param evt which will be passed from gecko external signal
 * @return void
 */
void gecko_update_client(struct gecko_cmd_packet* evt);

#endif
