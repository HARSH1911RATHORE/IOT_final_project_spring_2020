#include "i2c.h"

//bool event=false;                              //event flag to check intance of event occured


I2C_TransferReturn_TypeDef check_read;


/**
 * For disabling gpio pins in case of no event i.e load power management off
 *
 * @param void
 *
 */


void Gpio_sensor_enable()
{
	GPIO_PinModeSet(gpioPortD, 15, gpioModePushPull, true);                      //setting the i2c sensor enable pin
	GPIO_DriveStrengthSet(gpioPortD, gpioDriveStrengthStrongAlternateStrong);
}

void Gpio_disable()
{

	CORE_DECLARE_IRQ_STATE;                                                        //disabling interrupt at this point
	CORE_ENTER_CRITICAL();

	GPIO_PinModeSet(gpioPortC, SCL_pin, gpioModeDisabled, false);                  //scl gpio mode disabled
	GPIO_PinOutClear(gpioPortC, SCL_pin);                                          //clearing the i2c scl pin
	GPIO_PinModeSet(gpioPortC, SDA_pin, gpioModeDisabled, false);                  //sda gpio mode disabled
	GPIO_PinOutClear(gpioPortC, SDA_pin);                                          //clearing the i2c sda pin
	GPIO_PinOutClear(gpioPortD, 15);                                               //clearing sensor enable pin
	CORE_EXIT_CRITICAL();
}

/**
 * For enabling gpio pins in case of no event i.e load power management on
 *
 * @param void
 *
 */
void Gpio_enable()
{

	CORE_DECLARE_IRQ_STATE;                                                        //disabling interrupt at this point
	CORE_ENTER_CRITICAL();

	GPIO_PinModeSet(gpioPortC, SCL_pin, gpioModePushPull, false);                  //scl gpio mode disabled
	GPIO_DriveStrengthSet(gpioPortC, gpioDriveStrengthStrongAlternateStrong);

	GPIO_PinModeSet(gpioPortC, SDA_pin, gpioModePushPull, false);                  //sda gpio mode disabled

	CORE_EXIT_CRITICAL();
}

/**
 *Event handler to check if there is pending event to be handled
 *if event flag is true start i2c transfer otherwise disable gpio pin and load power management off
 * @param void
 *
 */

uint16_t event_handler()
{
	float read;
                                         //change the flag to false
		LOG_INFO("\nHUMID\n");
		init_i2c();                            //calling init i2c
		timerWaitUs(80000);                    //waiting for 80 ms power up time
		transfer_i2c();                        //write to i2c temperature sensor
		timerWaitUs(50000);                     //wait for some time for transfer
		read=read_temp_i2c();            //read the temperature
		LOG_INFO("Read rL humdity=%f",read);              //log the temperature
		event=0;

	return (uint16_t)read;

}

/**
 *  Initializing i2c pins, port, frequency, sensor enable pin etc.

	@param void

*/

void init_i2c()
{

	CORE_DECLARE_IRQ_STATE();                                               //critical section

                                            //critical section
	CORE_CRITICAL_IRQ_ENABLE();
	I2CSPM_Init_TypeDef init1=
	{       I2C0,                                                                  /* Use I2C instance 0 */
		    gpioPortC,                                                             /* SCL port */
			SCL_pin,                                                               /* SCL pin */
		    gpioPortC,                                                             /* SDA port */
			SDA_pin,                                                               /* SDA pin */
			SCL_Location,                                                          /* Location of SCL */
			SDA_Location,                                                          /* Location of SDA */
			Reference_clock,                                                       /* Use currently configured reference clock */
		    I2C_FREQ_STANDARD_MAX,                                                 /* Set to standard rate  */
		    i2cClockHLRStandard,                                                   /* Set to use 4:4 low/high duty cycle */
	};
	I2CSPM_Init( &init1);                                                          //calling i2c init

}


//~~~~~~~~~~~~~~~
void init_aqi_i2c()
{


		I2CSPM_Init_TypeDef init1=
		{       I2C0,                                                                  /* Use I2C instance 0 */
			    gpioPortD,                                                             /* SCL port */
				13,                                                               /* SCL pin */
			    gpioPortD,                                                             /* SDA port */
				14,                                                               /* SDA pin */
				20,                                                          /* Location of SCL */
				22,                                                          /* Location of SDA */
				Reference_clock,                                                       /* Use currently configured reference clock */
			    I2C_FREQ_STANDARD_MAX,                                                 /* Set to standard rate  */
			    i2cClockHLRStandard,                                                   /* Set to use 4:4 low/high duty cycle */
		};
		I2CSPM_Init( &init1);                                                          //calling i2c init


}
//~~~~~~~~~~~~~


/**
 * I2C transfer function to write data to slave address
 *
 * @param void
 *
 */

void transfer_i2c()
{
	I2C_TransferReturn_TypeDef check;                                              //variable to check the return status of transfer
	I2C_TransferSeq_TypeDef seq;                                                   //defining transfer variable
	seq.addr = slave_address << 1;                                                 //shifting the slave address by one
	seq.flags = I2C_FLAG_WRITE;													   //i2c write mode
	seq.buf[0].data = &no_hold_master;                                                 //selecting no hold master mode
	seq.buf[0].len = one_byte;                                                     //writing one byte of data

	check=I2CSPM_Transfer(I2C0,&seq);                                              //checking transfer status

	if (check!=i2cTransferDone)                                                    //if transfer not done print error status
	{
		LOG_ERROR("Failed to write %u bytes, return value was %d",one_byte,check);
		return;
	}
	LOG_INFO("SUCCESS");                                                           //else print success

}



/**
 * I2C read function to read the value of temperature from the slave by the master
 *
 * @param void
 * @return- temperature value in float
 *
 */
float read_temp_i2c()
{
	I2C_TransferReturn_TypeDef check_read;                                    //variable to check the return status of read
	I2C_TransferSeq_TypeDef seq_read;                                         //defining read variable
	uint8_t read_data[2]={0};                                                 //defining read buffer where temperature will be read
	uint16_t read_temp;                                                       //variable where read data will be passes
	seq_read.addr = slave_address << 1;                                       //shifting the slave address by one
	seq_read.flags = I2C_FLAG_READ;                                           //i2c read mode
	seq_read.buf[0].data = read_data;                                         //passing read data buffer
	seq_read.buf[0].len = 2;                                                  //specifying length of read data 2 bytes
	check_read=I2CSPM_Transfer(I2C0,&seq_read);                               //checking read status

	if (check_read!=i2cTransferDone)                                          //if transfer not done log error status
	{
		LOG_ERROR("Failed to read 2 bytes, return value was %d",check_read);
	}
	LOG_INFO("READ");                                                         //log read
	read_temp=read_data[0]<<8;                                                //left shift the read data buffer by 8 bits
	read_temp=read_data[1]|read_temp;                                         //exor read data with left shifted 8 bits
	float temp_val = ((175.72*read_temp)/65536)-46.85;                        //calculate temperature
	LOG_INFO("temperature_val: %f",temp_val);                                 //log temperature value
	return temp_val;                                                          //return temperature value

}

uint16_t READ_DATA_I2C(I2C_TransferSeq_TypeDef structure_init, uint8_t len)
{
	I2C_TransferReturn_TypeDef check;
	uint8_t received_aqi[2] = {0};			//receiving data
	uint16_t read_data;
//	uint64_t read_data_tvoc;
	structure_init.addr = 0x5A << 1;
	structure_init.flags= I2C_FLAG_READ;

	structure_init.buf[0].data=&received_aqi;
	structure_init.buf[0].len=len;
	check=I2CSPM_Transfer(I2C0,&structure_init);

	if(check != i2cTransferDone)
	{
		LOG_INFO("I2c Transaction in progress");
	}
	if(len==1)
	{
		read_data = received_aqi[0];
		return read_data;
	}
	else if(len==2)
	{


		read_data = received_aqi[0];

		read_data <<= 8;

		read_data	|=(received_aqi[1]);


	}

	return read_data;

}

void WRITE_DATA_I2C(I2C_TransferSeq_TypeDef init,uint16_t len)
{
	{
		I2C_TransferReturn_TypeDef ret;
		init.buf[0].len=len;
		init.flags= I2C_FLAG_WRITE;
		ret=I2CSPM_Transfer(I2C0,&init);
		if(ret != i2cTransferDone)
		{
			LOG_ERROR("I2C Write error");
			return;
		}
	}
}






