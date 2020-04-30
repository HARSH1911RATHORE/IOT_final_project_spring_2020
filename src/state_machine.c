//reference https://github.com/CU-ECEN-5823/course-project-PuneetBansal


#include "state_machine.h"
#include "main.h"
#include "display.h"

#include "infrastructure.h"

#include "gatt_db.h"

int sleep_m=0;                               //sleep mode decision variable

extern float read;

extern int bluetooth;						//bluetooth variable to check for the timer event interrupt that has occured like 80 ms, 10 ms, 15ms
extern int bluetooth_i2c;					//bluetooth i2c varibale check for the i2c event occured in the interrupt like read or write

extern int aqi_state;

extern int humid;
/**
 * state machine of i2c for switching states as the events complete
 * @param -none
 * return- void
 */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint16_t Maximum_humidity=0;
uint16_t Maximum_aqi=0;

#define Aqi_address 0x5A
#define STATUS_REGISTER 0x00
#define FIELD_APP_VALID (2^4)
#define APP_START_REGISTER (0xF4)
uint16_t FW_MODE_REGISTER=(2^7);
uint16_t MEAS_MODE_REGISTER=(0X01);

I2C_TransferSeq_TypeDef aqi_sensor;

uint8_t status_reg=0x00;
uint8_t HW_id=0x20;
uint8_t error_id=0xE0;

#define APP_VALID 1<<4
#define DATA_READY 1<<3
#define FW_MODE 1<<7
#define ALG_RES 0X02
//extern uint8_t write_data;
#define ALG_RESULT_DATA  0x02;

//I2C_TransferSeq_TypeDef aqi_sensor_change_mode;

extern bool event_write_aqi_done;
extern bool event_write_aqi_progressing;
bool event_configure_aqi=false;

uint8_t humid_STATE=0;
uint8_t aqi_STATE=0;
extern uint8_t received_data_aqi[2];
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void PersistentData(uint16_t ps_key , uint16_t value)
{
	int store_result=gecko_cmd_flash_ps_save(ps_key, sizeof(value),(const uint8_t*)&value)->result;
	if (store_result==0)
		LOG_INFO("\nSUCCESS in persistent data storage\n");
	else
		LOG_INFO("\nFAILURE  in persistent data storage\n");
}



void HUMIDITY_POLL()
{
	uint16_t read_hum=HUMIDITY_POLLING_EVENT();
	if(read_hum>Maximum_humidity)
	{
		Maximum_humidity=(uint16_t)read;
		PersistentData(Humidity_key,Maximum_humidity);
	}
}

void aqi_sensor_init()
{
	I2C_TransferReturn_TypeDef check;

	aqi_sensor.addr= 0x5A<<1;
	aqi_sensor.flags = I2C_FLAG_WRITE;
	aqi_sensor.buf[0].data=&HW_id;
	aqi_sensor.buf[0].len = one_byte;
	check=I2CSPM_Transfer(I2C0,&aqi_sensor);                                              //checking transfer status

	if (check!=i2cTransferDone)                                                    //if transfer not done print error status
	{
		LOG_ERROR("Failed to write %u bytes, return value was %d",one_byte,check);

	}
	else
	{
		LOG_INFO("SUCCESS in Initial write transfer");
	}
	uint8_t read_val_hw_id;
	uint8_t aqi=aqi_read;
	read_val_hw_id=READ_DATA_I2C(aqi_sensor,1,aqi);
	LOG_INFO("Read_data = %x\n",read_val_hw_id);
	uint8_t read_val;
	aqi_sensor.addr= 0x5A<<1;
	aqi_sensor.flags = I2C_FLAG_WRITE;
	aqi_sensor.buf[0].data=&status_reg;
	aqi_sensor.buf[0].len = one_byte;
	check=I2CSPM_Transfer(I2C0,&aqi_sensor);                                              //checking transfer status

	if (check!=i2cTransferDone)                                                    //if transfer not done print error status
	{
		LOG_ERROR("Failed to write %u bytes, return value was %d",one_byte,check);

	}
	else
	{
		LOG_INFO("SUCCESS in Initial write transfer");
	}
	read_val=READ_DATA_I2C(aqi_sensor,1,aqi);
	LOG_INFO("Status  Register = %x\n",read_val);
	I2C_TransferReturn_TypeDef check_ERROR_ID;
	uint8_t read_val_ERROR_ID;
	aqi_sensor.addr= 0x5A<<1;
	aqi_sensor.flags = I2C_FLAG_WRITE;
	aqi_sensor.buf[0].data=&error_id;
	aqi_sensor.buf[0].len = one_byte;
	check_ERROR_ID=I2CSPM_Transfer(I2C0,&aqi_sensor);                                              //checking transfer status

	if (check_ERROR_ID!=i2cTransferDone)                                                    //if transfer not done print error status
	{
		LOG_ERROR("Failed to write %u bytes, return value was %d",one_byte,check);

	}
	else
	{
		LOG_INFO("SUCCESS in writing ERROR ID");
	}

		read_val_ERROR_ID=READ_DATA_I2C(aqi_sensor,1,aqi);
		LOG_INFO("Status  Register = %x\n",read_val_ERROR_ID);

	if(read_val & FIELD_APP_VALID)
	{
		LOG_INFO("APP_VALID 1: Valid application firmware has been loaded\n");
	}
	uint8_t mode=APP_START_REGISTER;

	aqi_sensor.buf[0].data=&mode;

	WRITE_DATA_I2C(aqi_sensor,1);

	LOG_INFO("Status register monitored again \n");

	aqi_sensor.buf[0].data=&status_reg;

	WRITE_DATA_I2C(aqi_sensor,1);
	timerWaitUs(10000);
	read_val=READ_DATA_I2C(aqi_sensor,1,aqi);
	LOG_INFO("Status  Register = %x\n",read_val);

	if(read_val & 1<<7)
	{
		LOG_INFO("firmware mode\n");
	}
	//  Mode 1: Constant power mode, IAQ measurement every second, WRITING TO MEAS REGISTER
	aqi_sensor.addr= Aqi_address << 1;
	uint8_t cmd[2]={ 0x01, 0x10}; //This is Single byte register, which is used to enable sensor drive mode and interrupts.

	aqi_sensor.buf[0].data=cmd;
	WRITE_DATA_I2C(aqi_sensor,2);
	
	timerWaitUs(10000);

	cmd[0]=MEAS_MODE_REGISTER;
	aqi_sensor.buf[0].data=cmd;
	WRITE_DATA_I2C(aqi_sensor,1);
	timerWaitUs(10000);
	uint16_t value_read;

	value_read=READ_DATA_I2C(aqi_sensor,1,aqi);
	LOG_INFO("MEAS MODE REGISTER = %x\n",value_read);

}

void ppm_poll()
{
	if(humid_STATE!=1)
	{
		LOG_INFO("\nAQI\n");
		aqi_STATE=1;

		uint16_t status_get_aqi;
		uint16_t aqi_ppm;

		aqi_sensor.addr= Aqi_address << 1;
		aqi_sensor.buf[0].data=&status_reg;

		WRITE_DATA_I2C(aqi_sensor,1);
		timerWaitUs(10000);
	
		uint8_t aqi=aqi_read;
		status_get_aqi=READ_DATA_I2C(aqi_sensor,1,aqi);
		LOG_INFO("STATUS REGISTER=%x\n",status_get_aqi);
		if(status_get_aqi & DATA_READY)
		{
			aqi_sensor.addr= Aqi_address << 1;
			uint16_t command=ALG_RES;
			aqi_sensor.buf[0].data=&command;

			WRITE_DATA_I2C(aqi_sensor,1);

			timerWaitUs(1000000);
			aqi_ppm=READ_DATA_I2C(aqi_sensor,2,aqi);
			displayPrintf(DISPLAY_ROW_ACTION,"AQI: %d",aqi_ppm);
			if(aqi_ppm>Maximum_aqi)
			{
				Maximum_aqi=aqi_ppm;
				PersistentData(Air_quality_index_key,Maximum_aqi);
			}
			/* Send indication of the temperature in htmTempBuffer to all "listening" clients.
			* This enables the Health Thermometer in the Blue Gecko app to display the temperature.
			*  0xFF as connection ID will send indications to all connections. */
			gecko_cmd_gatt_server_send_characteristic_notification(connection_all, gattdb_pollen_concentration, 2, (const uint8*)&aqi_ppm);
			LOG_INFO("PPM POLL VALUE %d\n",aqi_ppm);
		}
		aqi_STATE=0;
	}
}



///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void state_machine_i2c_aqi()
{

	if (humid_STATE!=1)
	{
		aqi_STATE=1;
		switch(current_state_aqi)
		{
			case configure:
			{
				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();
				LOG_INFO("POWERING AQI I2C ON");                                                     //logging i2c power on in progress
				if (event_configure_aqi==true)																 //if underflow interrupt occurs i2c operations done
				{			
					event_configure_aqi=false;	
					write_data=ALG_RESULT_DATA;
					WRITE_READ_AQI(aqi_sensor,1,2);									//initialising i2c															
					current_state_aqi=read_aqi;                                     //current state changed to write i2c
				}
				else
				{
					event_configure_aqi=false;
					current_state_aqi=configure;
					NVIC_DisableIRQ(I2C0_IRQn);
				}
			CORE_CRITICAL_IRQ_ENABLE();


			break;
			}	

			case read_aqi:
			{
				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();
				if(event_write_aqi_done==true)														//if 80 ms complete
				{
			
					event_write_aqi_done=false;
					
					LOG_INFO("value in data buffer aqi[0] is %d",received_data_aqi[0]);
					LOG_INFO("value in data buffer aqi[1] is %d",received_data_aqi[1]);
					uint16_t aqi_ppm=received_data_aqi[0]<<8;
					aqi_ppm=aqi_ppm|received_data_aqi[1];

					/* Send indication of the temperature in htmTempBuffer to all "listening" clients.
					 * This enables the Health Thermometer in the Blue Gecko app to display the temperature.
					 *  0xFF as connection ID will send indications to all connections. */
					gecko_cmd_gatt_server_send_characteristic_notification(
							connection_all, gattdb_pollen_concentration, 2, (const uint8*)&aqi_ppm);

					LOG_INFO("value in data buffer aqi[0] is %d",received_data_aqi[0]);
					LOG_INFO("value in data buffer aqi[1] is %d",received_data_aqi[1]);
					LOG_INFO("PPM VALUE=%d",aqi_ppm);
					displayPrintf(DISPLAY_ROW_ACTION,"AQI: %d",aqi_ppm);
					if(aqi_ppm>Maximum_aqi)
					{
						Maximum_aqi=aqi_ppm;
						PersistentData(Air_quality_index_key,Maximum_aqi);
					}
					LOG_INFO("LOG write_aqi_done");

					current_state=configure;											//current state made read
				}
				else if(event_write_aqi_progressing==true)
				{		
					event_write_aqi_progressing=false;
					LOG_INFO("LOG write_aqi_progressing ");
					aqi_state=0;

					current_state=read_aqi;
				}
				aqi_STATE=0;

				CORE_CRITICAL_IRQ_ENABLE();
				break;
			}
		}

	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void state_machine_i2c_humidity()
{
	if (aqi_STATE!=1)
	{
	humid_STATE=1;
	switch(current_state)
	{
		case power_up:
		{
			CORE_DECLARE_IRQ_STATE();
			CORE_CRITICAL_IRQ_DISABLE();
			LOG_INFO("POWERING I2C ON");                                                     //logging i2c power on in progress
			if (event==true)																 //if underflow interrupt occurs i2c operations done
			{
				bluetooth=timer_80;
				sleep_m=1;																	 //sleep mode device should be in                                                 //critical section
				event=false;															     //event made false				
				GPIO_PinModeSet(gpioPortD, 15, gpioModePushPull, true);                      //setting the i2c sensor enable pin
				GPIO_DriveStrengthSet(gpioPortD, gpioDriveStrengthStrongAlternateStrong);
				timerSetEventInMs(80);														 //80 mili second power up time
				LOG_INFO("I2C INIT BEGIN");
				init_i2c();																	//initialising i2c

				current_state=wait_for_i2c_write_start;                                     //current state changed to write i2c
			}
			CORE_CRITICAL_IRQ_ENABLE();
			break;
		}
		case wait_for_i2c_write_start:
		{ 
			CORE_DECLARE_IRQ_STATE();
			CORE_CRITICAL_IRQ_DISABLE();
			if(event_timer_80==true)														//if 80 ms complete
			{
				bluetooth=timer_10;
				bluetooth_i2c=write_state;
				event_timer_80=false;														//event flag made fals				
				LOG_INFO("I2C WRITE BEGIN");												//log write
				SLEEP_SleepBlockBegin(sleepEM2);											//to start sleep mode em2
				if (conn_open_id==1)														//if bluetooth connection open, then i2c transfer occurs
				{
				I2C_transfer_interrupt_init();												//start i2c transfer sequence
				timerSetEventInMs(10);
				current_state=wait_for_i2c_read_start;											//current state made read
				}
			}
			CORE_CRITICAL_IRQ_ENABLE();
			break;
		}

		case wait_for_i2c_read_start:
		{
			CORE_DECLARE_IRQ_STATE();
			CORE_CRITICAL_IRQ_DISABLE();
			if(event_write==true )															//event write complete
			{
				bluetooth=timer_5;
				bluetooth_i2c=read_state													//bluetooth i2c interrupt occurs because of read now
				CORE_DECLARE_IRQ_STATE();													//critical section
				CORE_CRITICAL_IRQ_DISABLE();
				event_write=false;
																							//event flag made false
				CORE_CRITICAL_IRQ_ENABLE();
				LOG_INFO("I2C READ BEGIN");
				I2C_read_interrupt();														//read i2c
				timerSetEventInMs(10);
				current_state=display;												//state changed to display
			}
			else if (event_i2c_progressing_write==true)
			{
				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();
				event_i2c_progressing_write=false;
				SLEEP_SleepBlockEnd(sleepEM2);
				CORE_CRITICAL_IRQ_ENABLE();
				LOG_INFO("I2c in progress stage for write");
			}
			CORE_CRITICAL_IRQ_ENABLE();
			break;
		}

		case display:
		{
			CORE_DECLARE_IRQ_STATE();
			CORE_CRITICAL_IRQ_DISABLE();
			if(event_read==true )															//if read interrupt complete
			{

				event_read=false;															//event read flag made false

				LOG_INFO("I2C READ");														//log i2c reading state
				read=I2C_Read_humidity_value();            										//read the temperature
				if(read>Maximum_humidity)
				{
					Maximum_humidity=(uint16_t)read;
					PersistentData(Humidity_key,Maximum_humidity);
				}
				LOG_INFO("read_humidity=%f",read);
				humidityMeasure();
				displayPrintf(DISPLAY_ROW_TEMPVALUE,"HUMIDITY=%.2f RH",read);
				SLEEP_SleepBlockEnd(sleepEM2);												//to end sleep mode
				if (lcd==false)
					Gpio_disable();																//turn the pins of i2c off
				current_state=sleeping;
			}
			else if (event_i2c_progressing_read==true)
			{
				event_i2c_progressing_read=false;
				SLEEP_SleepBlockEnd(sleepEM2);
				LOG_INFO("I2c in progress stage for read");
			}
			CORE_CRITICAL_IRQ_ENABLE();
			break;
		}
		case sleeping:
		{
			CORE_DECLARE_IRQ_STATE();
			CORE_CRITICAL_IRQ_DISABLE();
			sleep_m=0;
			current_state=configure;
			event_configure_aqi=true;
			humid_STATE=0;
			current_state=power_up;															//current state made power up
		}
			CORE_CRITICAL_IRQ_ENABLE();
			break;

	}
	}

}



