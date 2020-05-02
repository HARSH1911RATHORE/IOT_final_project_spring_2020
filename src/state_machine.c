//@reference - https://cdn.sparkfun.com/datasheets/BreakoutBoards/CCS811_Programming_Guide.pdf
//@reference - https://github.com/CU-ECEN-5823/course-project-PuneetBansal
//@reference - SI LABS API



#include "state_machine.h"
#include "main.h"
#include "display.h"

int sleep_m=0;                               //sleep mode decision variable

extern float read;

extern int bluetooth;						//bluetooth variable to check for the timer event interrupt that has occured like 80 ms, 10 ms, 15ms
extern int bluetooth_i2c;					//bluetooth i2c varibale check for the i2c event occured in the interrupt like read or write


/**
 * state machine of i2c for switching states as the events complete
 * @param -none
 * return- void
 */

uint16_t Maximum_humidity=0;			//keep a track of max humidity and aqi value
uint16_t Maximum_aqi=0;

#define Aqi_address 0x5A				//aqi slave address
#define STATUS_REGISTER 0x00			//aqi status register

uint16_t APP_START_REGISTER=(0xF4);		//app start register command start the sensor in application mode
uint16_t FW_MODE_REGISTER=(2^7);		//bit field inside status register to see if sensor is in firmware mode
uint16_t MEAS_MODE_REGISTER=(0X01);		//meas mode register to set how reading will be obtained

I2C_TransferSeq_TypeDef aqi_sensor;

uint8_t write=0x00;						//status register value
uint8_t HW_id=0x20;						//hardware id to see if it is the valid sensor
uint8_t Software_reset=0xFF;			//to perform software reset
uint8_t error_id=0xE0;					//error id register to see if there are any errors
#define APP_VALID 1<<4					//bit field app valid in status register
#define DATA_READY 1<<3
#define FW_MODE 1<<7					//bit field inside status register to see if sensor is in firmware mode
#define ALG_RES 0X02					//algorithmic result, msb being co2c pppm

bool event_write_aqi_done=false;		//write event for aqi done
bool event_write_aqi_progressing=false; //write event for aqi progressing
bool event_configure_aqi=false;         //aqi configure event

uint8_t humid_STATE_MACHINE=0;			//humidity state machine active
uint8_t aqi_STATE_MACHINE=0;			//aqi state machine active
extern uint8_t received_data_aqi[2];	//receive value of aqi ppm

void aqi_sensor_init()
{

	I2C_TransferReturn_TypeDef check;

	aqi_sensor.addr= 0x5A<<1;														//The sensor hardware id is checked to see if it is the correct sensor
	aqi_sensor.flags = I2C_FLAG_WRITE;
	aqi_sensor.buf[0].data=&HW_id;
	aqi_sensor.buf[0].len = one_byte;
	check=I2CSPM_Transfer(I2C0,&aqi_sensor);                                       //checking transfer status

	if (check!=i2cTransferDone)                                                    //if transfer not done print error status
	{
		LOG_ERROR("Failed to write %u bytes, return value was %d",one_byte,check);

	}
	else
	{
		LOG_INFO("SUCCESS in Initial write transfer");
	}
	uint8_t read_val_hw_id;
	read_val_hw_id=READ_DATA_I2C(aqi_sensor,1);
	LOG_INFO("Read_data = %x\n",read_val_hw_id);										//hw id read should be 81
	uint8_t read_val;
	aqi_sensor.addr= 0x5A<<1;
	aqi_sensor.flags = I2C_FLAG_WRITE;													//checking for the the status register value
	aqi_sensor.buf[0].data=&write;
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

	read_val=READ_DATA_I2C(aqi_sensor,1);
	LOG_INFO("Status  Register = %x\n",read_val);
	I2C_TransferReturn_TypeDef check_ERROR_ID;										//checking for any error, if there are no errors obtained from the sensor the sensor will read 0
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
		read_val_ERROR_ID=READ_DATA_I2C(aqi_sensor,1);
		LOG_INFO("Status  Register = %x\n",read_val_ERROR_ID);

	if(read_val & APP_VALID)																//checking to see if valid application has been loaded
	{
		LOG_INFO("APP_VALID 1: Valid application firmware has been loaded\n");
	}
	uint8_t mode=APP_START_REGISTER;												//setup write to application start

	aqi_sensor.buf[0].data=&mode;

	WRITE_DATA_I2C(aqi_sensor,1);


	LOG_INFO("Status register monitored again \n");


	aqi_sensor.buf[0].data=&write;													//checking the status register again

	WRITE_DATA_I2C(aqi_sensor,1);


	read_val=READ_DATA_I2C(aqi_sensor,1);
	LOG_INFO("Status  Register = %x\n",read_val);

	if(read_val & FW_MODE)
	{
		LOG_INFO("firmware mode\n");											//if msb set in status register, sensor is in firmware mode
	}


	//  Mode 1: Constant power mode, IAQ measurement every second, WRITING TO MEAS REGISTER
	I2C_TransferSeq_TypeDef aqi_sensor_change_mode;

	aqi_sensor_change_mode.addr= Aqi_address << 1;
	uint8_t cmd[2]={ 0x01, 0x18}; 				//This is Single byte register, which is used to enable sensor drive mode and interrupts.

	aqi_sensor_change_mode.buf[0].data=cmd;
	WRITE_DATA_I2C(aqi_sensor_change_mode,2);


	cmd[0]=MEAS_MODE_REGISTER;								//check to see the value in measure mode register for the data drive mode bit and interrupt bit
	aqi_sensor_change_mode.buf[0].data=cmd;
	WRITE_DATA_I2C(aqi_sensor_change_mode,1);

	uint16_t value_read;
	value_read=READ_DATA_I2C(aqi_sensor_change_mode,1);
	LOG_INFO("MEAS MODE REGISTER = %x\n",value_read);
	I2C_TransferSeq_TypeDef init;
	uint16_t status_get;

	init.addr= Aqi_address << 1;											//monitoring status register again

	init.buf[0].data=&write;

	WRITE_DATA_I2C(init,1);


	status_get=READ_DATA_I2C(init,1);
	LOG_INFO("STATUS REGISTER=%x\n",status_get);
	if(status_get & DATA_READY)												//if status register 4th bit is set, the data is ready
	{
		init.addr= Aqi_address << 1;
		uint16_t command=ALG_RES;
		init.buf[0].data=&command;

		WRITE_DATA_I2C(init,1);

	//	for (long i=0;i<100000000000;i++)

		status_get=READ_DATA_I2C(init,2);
		LOG_INFO("value in data buffer aqi[0] is %d",received_data_aqi[0]);
		LOG_INFO("value in data buffer aqi[1] is %d",received_data_aqi[1]);
		LOG_INFO("ppm value is %d\n",status_get);
	}
}

void ppm_poll()
{
	I2C_TransferSeq_TypeDef ppm_init;
	uint16_t status_get_aqi;
	status_get_aqi=READ_DATA_I2C(ppm_init,1);
	LOG_INFO("STATUS REGISTER=%d\n",status_get_aqi);
	if(status_get_aqi & DATA_READY)
	{
		ppm_init.addr= Aqi_address << 1;
		uint16_t command=ALG_RES;
		ppm_init.buf[0].data=&command;

		WRITE_DATA_I2C(ppm_init,1);

//		timerWaitUs(10000);
		status_get_aqi=READ_DATA_I2C(ppm_init,2);
		LOG_INFO("value in data buffer aqi[0] is %d",received_data_aqi[0]);
		LOG_INFO("value in data buffer aqi[1] is %d",received_data_aqi[1]);
		LOG_INFO("ppm value is %d\n",status_get_aqi);
	}
}

void PersistentData(uint16_t ps_key , uint16_t value)
{
	int store_result=gecko_cmd_flash_ps_save(ps_key, sizeof(value),(const uint8_t*)&value)->result;
	if (store_result==0)
		LOG_INFO("\nSUCCESS in persistent data storage\n");
	else
		LOG_INFO("\nFAILURE  in persistent data storage\n");
}



void state_machine_i2c_aqi()												//aqi state machine
{
	CORE_ATOMIC_IRQ_DISABLE();
	if (humid_STATE_MACHINE!=1)
	{
		aqi_STATE_MACHINE=1;
		switch(current_state_aqi)
		{
			case configure:
			{
				LOG_INFO("POWERING AQI I2C ON");                                     //logging i2c power on in progress
				if (event_configure_aqi==true)										//if underflow interrupt occurs i2c operations done
				{
					LOG_INFO("event_configure_aqi==true");
					CORE_DECLARE_IRQ_STATE();
					CORE_CRITICAL_IRQ_DISABLE();
					event_configure_aqi=false;										//configure event made false
					CORE_CRITICAL_IRQ_ENABLE();
					WRITE_READ_AQI(1,2);											//write read the alg result command to obatin the ppm value

					current_state_aqi=read_aqi;                                     //current state changed to read i2c
				}
				else
				{
					LOG_INFO("event_configure_aqi else");
					current_state_aqi=configure;

				}
				CORE_ATOMIC_IRQ_ENABLE();
				break;
			}
			case read_aqi:
			{
				CORE_ATOMIC_IRQ_DISABLE();
				if(event_write_aqi_done==true)								//if write aqi done
				{
					LOG_INFO("event_write_aqi_done==true");
					CORE_DECLARE_IRQ_STATE();
					CORE_CRITICAL_IRQ_DISABLE();
					event_write_aqi_done=false;
					CORE_CRITICAL_IRQ_ENABLE();								//values are stored in big endian format
					uint16_t aqi_ppm=received_data_aqi[0]<<8;				//changing the register data such that correct reading is read in a little endian architecture
					aqi_ppm=aqi_ppm|received_data_aqi[1];
					LOG_INFO("value in data buffer aqi[0] is %d",received_data_aqi[0]);
					LOG_INFO("value in data buffer aqi[1] is %d",received_data_aqi[1]);
					LOG_INFO("PPM VALUE=%d",aqi_ppm);
					displayPrintf(DISPLAY_ROW_MAX,"%d",aqi_ppm);
					displayPrintf(DISPLAY_ROW_ACTION,"PPM %d",aqi_ppm);
					if(aqi_ppm>Maximum_aqi)
					{														//if maximum value exceeded the data is stored in persistent memory
						Maximum_aqi=aqi_ppm;
						PersistentData(Air_quality_index_key,Maximum_aqi);
					}
					LOG_INFO("LOG 1");

					current_state=configure;								//current state made configure
				}
				else if(event_write_aqi_progressing==true)
				{
					LOG_INFO("event_write_aqi_progressing==true");
					CORE_DECLARE_IRQ_STATE();
					CORE_CRITICAL_IRQ_DISABLE();
					event_write_aqi_progressing=false;					//if i2c transaction not complete
					CORE_CRITICAL_IRQ_ENABLE();
					LOG_INFO("LOG 2");
					current_state=read_aqi;
				}
				aqi_STATE_MACHINE=0;

				CORE_ATOMIC_IRQ_DISABLE();
				break;
			}
		}
	}

}


void state_machine_i2c_humidity()
{

	switch(current_state)
	{
		case power_up:
		{
			LOG_INFO("POWERING I2C ON");                                                     //logging i2c power on in progress
			if (event==true)																 //if underflow interrupt occurs i2c operations done
			{
				bluetooth=timer_80;
				sleep_m=1;																	 //sleep mode device should be in

				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();                                                 //critical section
				event=false;															     //event made false
				CORE_CRITICAL_IRQ_ENABLE();
				GPIO_PinModeSet(gpioPortD, 15, gpioModePushPull, true);                      //setting the i2c sensor enable pin
				GPIO_DriveStrengthSet(gpioPortD, gpioDriveStrengthStrongAlternateStrong);
				timerSetEventInMs(80);														 //80 mili second power up time
				LOG_INFO("I2C INIT BEGIN");
				init_i2c();																	//initialising i2c

				current_state=wait_for_i2c_write_start;                                     //current state changed to write i2c
			}
			break;
		}
		case wait_for_i2c_write_start:
		{
			if(event_timer_80==true)														//if 80 ms complete
			{
				bluetooth=timer_10;
				bluetooth_i2c=write_state;
				CORE_DECLARE_IRQ_STATE();													//critical section
				CORE_CRITICAL_IRQ_DISABLE();
				event_timer_80=false;														//event flag made false

				CORE_CRITICAL_IRQ_ENABLE();
				LOG_INFO("I2C WRITE BEGIN");												//log write

				SLEEP_SleepBlockBegin(sleepEM2);											//to start sleep mode em2
				if (conn_open_id==1)														//if bluetooth connection open, then i2c transfer occurs
				{
				I2C_transfer_interrupt_init();												//start i2c transfer sequence
				timerSetEventInMs(10);
				current_state=wait_for_i2c_read_start;											//current state made read
				}
			}

			break;
		}

		case wait_for_i2c_read_start:
		{
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

			break;
		}

		case display:
		{
			if(event_read==true )															//if read interrupt complete
			{

				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();
				event_read=false;															//event read flag made false
				CORE_CRITICAL_IRQ_ENABLE();
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
				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();
				event_i2c_progressing_read=false;

				CORE_CRITICAL_IRQ_ENABLE();
				SLEEP_SleepBlockEnd(sleepEM2);
				LOG_INFO("I2c in progress stage for read");
			}

			break;
		}
		case sleeping:
		{
			sleep_m=0;

			current_state=power_up;															//current state made power up

		}
			break;

	}


}


