//@reference-SILICON LABS SOC THERMOMETER CLIENT EXAMPLE, SILICON LABS API, SILICON LABS SOC THERMOMETER EXAMPLE
//@reference-https://docs.silabs.com/bluetooth/latest/


#include "gecko_configuration.h"
#include "gpio.h"
#include "native_gecko.h"             //enabling all required libraries for low energy timer,i2c, logging, bluetooth and sleep routines
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
#include "Server.h"
#include "Client.h"

#include "../platform/emdrv/gpiointerrupt/inc/gpiointerrupt.h"


float read=0;
bool lcd =false;

bool event=false;							  //event 3 second letimer uf interrupt
bool event_write=false;						  //event write i2c
bool event_read=false;                        //event flags for write and read
bool event_i2c_progressing_write=false;		  //event i2c write in progress
bool event_i2c_progressing_read=false;	      //event i2c read in progress


int conn_open_id=0;
bool event_timer_80=false;                    //event flag 80 mili seconds
bool event_timer_10=false;                     //event flag 10 mili seconds
bool event_timer_5=false;

uint32_t event_bluetooth=1;                  //event bluetooth is 3 second uf interrupt
uint32_t event_timer_10_bluetooth=2;        //event timer 10 is 10 mili second after i2c transfer
uint32_t event_timer_80_bluetooth=4;		//event timer 80 is 80 mili second startup event
uint32_t event_timer_5_bluetooth=8;			//event timer 5 is 5 mili second
uint32_t event_read_bluetooth=16;           //event read is read interrupt event
uint32_t event_write_bluetooth=32;			//event 32 is event write event

uint32_t event_i2c_progress_write=64;       //event progress write is write in progress
uint32_t event_i2c_progress_read=128;		//event progress read is read in progress
//~~~~~~~~
uint32_t event_gpio_Callback=1<<11;
uint32_t gpio_call=0;
//~~~~~~~
/**
* @return a float value based on a UINT32 value written by FLT_TO_UINT32
and
* UINT32_TO_BITSTREAM
* @param value_start_little_endian is a pointer to the first byte of the
float
* which is represented in UINT32 format from
FLT_TO_UINT32/UINT32_TO_BITSTREAM
*/
float gattUint32ToFloat(const uint8_t *value_start_little_endian)
{
	int8_t exponent = (int8_t)value_start_little_endian[3];
	uint32_t mantissa = value_start_little_endian[0] +(((uint32_t)value_start_little_endian[1]) << 8) +(((uint32_t)value_start_little_endian[2]) << 16);
	return (float)mantissa*pow(10,exponent);
}


void sleep()
{
	const SLEEP_Init_t init = {0};                   // init structure value set to zero
	SLEEP_InitEx(&init);
}

/**
 * Temperature read which will be visible on app
 *
 * @param void
 *
 */
void humidityMeasure()
{
	uint8_t htmTempBuffer[5]; /* Stores the temperature data in the Health Thermometer (HTM) format. */
	uint8_t flags = 0x00;   /* HTM flags set as 0 for Celsius, no time stamp and no temperature type. */


	uint32_t temperature;   /* Stores the temperature data read from the sensor in the correct format */
	uint8_t *p = htmTempBuffer; /* Pointer to HTM temperature buffer needed for converting values to bitstream. */


	/* Convert flags to bitstream and append them in the HTM temperature data buffer (htmTempBuffer) */
	UINT8_TO_BITSTREAM(p, flags);


	/* Convert sensor data to correct temperature format */
	temperature = FLT_TO_UINT32(read*1000, -3);
	/* Convert temperature to bitstream and place it in the HTM temperature data buffer (htmTempBuffer) */
	UINT32_TO_BITSTREAM(p, temperature);

	/* Send indication of the temperature in htmTempBuffer to all "listening" clients.
	 * This enables the Health Thermometer in the Blue Gecko app to display the temperature.
	 *  0xFF as connection ID will send indications to all connections. */
	gecko_cmd_gatt_server_send_characteristic_notification(
			connection_all, gattdb_humidity, characteristic_handle, htmTempBuffer);
	gecko_cmd_le_connection_get_rssi(val);
}

/**
 * shutdown i2c and letimer
 *
 * @param void
 *
 */
void shut_down_i2c_letimer()
{
	if (lcd==false)
		Gpio_disable();																	//shutdown i2c and letimer
	LETIMER_Reset(LETIMER0);

}
void gpioCallback1(uint8_t pin)
{
	LOG_INFO("gpio CALLBACK");
	gpio_call|=event_gpio_Callback;
	gecko_external_signal(gpio_call);

}

int appMain(gecko_configuration_t *config)
{
	gecko_init(config);
	gpioInit();                                      //gpio init
	gpioLed0SetOff();                                //setting led off initially
	clockInit();                                     //clock initialization for letimer
	logInit();
	displayInit();
	gpio_interrupt_enable();						//call gpio external interrupt init
	// Enable clock for GPIO module, initialize GPIOINT
	CMU_ClockEnable(cmuClock_GPIO, true);
	GPIOINT_Init();
	  /* configure interrupt for PB0 and PB1, rising edges */
	GPIO_ExtIntConfig(gpioPortA, 2, 10,
	                    true, true, true);
	// Register callback functions and enable interrupts
	GPIOINT_CallbackRegister(10, gpioCallback1);
	current_state=power_up;                         //setting current state to power up

	//~~
	init_aqi_i2c();
	aqi_sensor_init();
	//~~

	while(1)
	{
		if (IsClientDevice()==false)				//checking if client mode is on
		{
			state_machine_i2c_humidity();					//if not state machine works
			state_machine_i2c_aqi();
		}
		struct gecko_cmd_packet* evt;
		evt = gecko_wait_event();					//calling event wait
		if (IsClientDevice()==true)					//if client is true, call client function else call server
			gecko_update_client(evt);
		else
			gecko_ecen5823_update(evt);
	}
	return 0;
}
