#include "state_machine.h"
#include "main.h"
#include "display.h"
#include "i2c.h"
int sleep_m=0;                               //sleep mode decision variable

extern float read;

extern int bluetooth;						//bluetooth variable to check for the timer event interrupt that has occured like 80 ms, 10 ms, 15ms
extern int bluetooth_i2c;					//bluetooth i2c varibale check for the i2c event occured in the interrupt like read or write

#define Aqi_address 0x5A
uint8_t STATUS_REGISTER=0x00;
uint8_t FIELD_APP_VALID=(2^4);
uint8_t APP_START_REGISTER=(0xF4);
uint8_t FW_MODE_REGISTER=(2^7);
uint8_t MEAS_MODE_REGISTER=(0X01);

void aqi_sensor_init()
{
	I2C_TransferSeq_TypeDef aqi_sensor;
	size_t read_val;

	aqi_sensor.addr= Aqi_address << 1;
//	uint16_t mode=STATUS_REGISTER;
	aqi_sensor.buf[0].data=&STATUS_REGISTER;

	WRITE_DATA_I2C(aqi_sensor,1);
	timerWaitUs(80000);

	read_val=READ_DATA_I2C(aqi_sensor,1);
	LOG_INFO("Status  Register = %x\n",read_val);

	if(read_val & FIELD_APP_VALID)
	{
		LOG_INFO("APP_VALID 1: Valid application firmware has been loaded\n");
	}

//	mode=APP_START_REGISTER;
	aqi_sensor.buf[0].data=&APP_START_REGISTER;

	WRITE_DATA_I2C(aqi_sensor,1);
	timerWaitUs(80000);

	LOG_INFO("Status register monitored again \n");

//	mode=STATUS_REGISTER;
	aqi_sensor.buf[0].data=&STATUS_REGISTER;

	WRITE_DATA_I2C(aqi_sensor,1);
	timerWaitUs(10000);

	read_val=READ_DATA_I2C(aqi_sensor,1);
	LOG_INFO("Status  Register = %x\n",read_val);

	if(read_val & FW_MODE_REGISTER)
	{
		LOG_INFO("firmware mode\n");
	}

	//  Mode 1: Constant power mode, IAQ measurement every second, WRITING TO MEAS REGISTER
	I2C_TransferSeq_TypeDef aqi_sensor_change_mode;


	aqi_sensor_change_mode.addr= Aqi_address << 1;
	uint8_t cmd[2]={ 0x01, 0x18}; //This is Single byte register, which is used to enable sensor drive mode and interrupts.

	aqi_sensor_change_mode.buf[0].data=cmd;
	WRITE_DATA_I2C(aqi_sensor_change_mode,2);
	timerWaitUs(10000);

	cmd[0]=MEAS_MODE_REGISTER;
	aqi_sensor_change_mode.buf[0].data=cmd;
	WRITE_DATA_I2C(aqi_sensor_change_mode,1);
	timerWaitUs(80000);
	uint16_t value_read;
	value_read=READ_DATA_I2C(aqi_sensor_change_mode,1);
	LOG_INFO("MEAS MODE REGISTER = %x\n",value_read);
}
///////////////////////////////////
void state_machine_i2c_aqi_ccs811()
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
				read=I2C_Read_temp_value();            										//read the temperature
				LOG_INFO("read_temperature=%f",read);
				temperatureMeasure();
				displayPrintf(DISPLAY_ROW_TEMPVALUE,"TEMP=%f",read);
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
/////////////////////////
/**
 * state machine of i2c for switching states as the events complete
 * @param -none
 * return- void
 */
void state_machine_i2c_si7021()
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
				read=I2C_Read_temp_value();            										//read the temperature
				LOG_INFO("read_temperature=%f",read);
				temperatureMeasure();
				displayPrintf(DISPLAY_ROW_TEMPVALUE,"TEMP=%f",read);
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


