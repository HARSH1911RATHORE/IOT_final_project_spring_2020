//@reference - https://cdn.sparkfun.com/datasheets/BreakoutBoards/CCS811_Programming_Guide.pdf
//@reference - https://github.com/CU-ECEN-5823/course-project-PuneetBansal
//@reference - SI LABS API

#include "I2C_INTERRUPT_MODE.h"
#include "i2c.h"
#include "state_machine.h"

///////////////////////////////////////////Variables for i2c////////////////////////////
uint8_t no_hold_master=0xF5;													//setting no hold master mode
I2C_TransferSeq_TypeDef seq;												//transfer seq structure for read and write i2c
I2C_TransferSeq_TypeDef structure_init;
uint32_t one_byte=1;                                                        //one byte of write data
uint8_t read_data_int[2]={0};                                               //2 bytes of read buffer

bool event_i2c=false;														//checking for event_i2c interrupt event

uint32_t i2c_evt;															//checking the type of i2c interrupt event occured

int bluetooth_i2c=0;

uint8_t read_data_humid[2]={0};												//read data to store humidity value
uint8_t received_data_aqi[2]={0};											//read data to store aqi value
uint16_t event_aqi_wr_done=1<<10;											//event aqi wr done
uint16_t event_aqi_wr_progress=1<<11;										//event aqi wr in progress

uint8_t Data1=0xFF;															//aqi software reset id
uint8_t Data2[2]={0x11,0xE5};												//values for software reset
uint8_t Data3[2]={0x72,0x8A};
////////////////////////////////////////////////////////////////////////////////////////


/**
 * I2C display function to display and convert data from slave address
 *
 * @param float temp value
 *
 */

float I2C_Read_temp_value()
{
	uint16_t read_temp;
	LOG_INFO("READ");                                                             //log read
	read_temp=read_data_int[0]<<8;                                                //left shift the read data buffer by 8 bits
	read_temp=read_data_int[1]|read_temp;                                         //or read data with left shifted 8 bits
	float temp_val = ((175.72*read_temp)/65536)-46.85;                            //calculate temperature
	LOG_INFO("temperature_val: %f",temp_val);                                     //log temperature value

	return temp_val;
}

/**
 * I2C interrupt handler which handles i2c interrupt
 *
 * @param void
 *
 */

void I2C_transfer_interrupt_init()
{
	seq.addr=slave_address<<1;                                              //shifting the slave address by one
	seq.flags=I2C_FLAG_WRITE;                                               //i2c write mode
	seq.buf[0].data=&no_hold_master;                                            //selecting no hold master mode
	seq.buf[0].len=one_byte;                                                //writing one byte of data
	NVIC_EnableIRQ(I2C0_IRQn);                                              //enabling i2c interrupt
	I2C_Enable(I2C0, true);                                                 //i2c0 enable
	I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C0,&seq);
	if(ret != i2cTransferDone)                                              //if transfer not done print transfer in progress
	{
		LOG_ERROR("Transfer in progress");
	}
}
/**
 * I2C read function to read data for  slave address
 *
 * @param void
 *
 */

void I2C_read_interrupt()
{

	seq.addr=slave_address<<1;                                                   //shifting the slave address by one
	seq.flags=I2C_FLAG_READ;                                                     //i2c read mode
	seq.buf[0].data=read_data_humid;                                               //defining read buffer where temperature will be read
	seq.buf[0].len=2;                                                            //writing two bytes of data
	I2C_TransferReturn_TypeDef check_read_int = I2C_TransferInit(I2C0,&seq);

	if(check_read_int != i2cTransferDone)                                		 //if transfer not done print transfer in progress
	{
		LOG_ERROR("Transfer in progress");
	}
}

/**
 * I2C read function value conversion to get humidity
 *
 * @param void
 *
 */
float I2C_Read_humidity_value()
{
	uint16_t read_humidity;
	LOG_INFO("READ");                                                             //log read
	read_humidity=read_data_humid[0]<<8;                                          //left shift the read data buffer by 8 bits
	read_humidity=read_data_humid[1]|read_humidity;                              //or read data with left shifted 8 bits
	float rl_humidity = (((read_data_humid[0] * 256 + read_data_humid[1]) * 125.0) / 65536.0) - 6;
	LOG_INFO("Relative Humidity : %.2f RH \n", rl_humidity);
	return rl_humidity;
}

/**
 * I2C write read
 *
 *  @param write length and read length
 *  @return - void
 */
void WRITE_READ_AQI( uint8_t len_write,uint8_t len_read )
{
	structure_init.addr = 0x5A << 1;
	structure_init.flags= I2C_FLAG_WRITE_READ;
	uint8_t cmd=ALG_RESULT_DATA;
	structure_init.buf[0].data= &cmd;										//Algorithm result
	structure_init.buf[0].len=len_write;
	structure_init.buf[1].data= received_data_aqi;							//data stored in received buffer
	structure_init.buf[1].len=len_read;
	NVIC_EnableIRQ(I2C0_IRQn);                                              //enabling i2c interrupt
	I2C_Enable(I2C0, true);                                                 //i2c0 enable
	I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C0,&structure_init);
	if(ret != i2cTransferDone)                                              //if transfer not done print transfer in progress
	{
		LOG_ERROR(" Progress: Write and read using flag write read  ");
	}
	else
	{
		LOG_INFO("AQI DONE WRITE READ $$$$$$$$");
	}

}


/**
 * I2C write write
 *
 *  @param void
 *  @return - void
 */
void WriteWrite_int_i2c()
{
	I2C_TransferSeq_TypeDef init;
	init.addr= 0X5A << 1;												//slave address of aqi
	init.flags= I2C_FLAG_WRITE_WRITE;
	init.buf[0].len=2; // setting the length of write buffer
	init.buf[1].len=2;
	init.buf[0].data= Data2;											//data being written are bytes required for software reset of sensor
	init.buf[1].data= Data3;
	LOG_INFO("Buffer 0 data  is %x",init.buf[0].data[0]);
	LOG_INFO("Buffer 1 data  is %x",init.buf[1].data[0]);

	I2C_TransferReturn_TypeDef ret;

	ret=I2CSPM_Transfer(I2C0,&init);									//transfer mode
	if(ret != i2cTransferDone)
	{
		LOG_ERROR("I2C Write error");
		return;
	}


}


/**
 * I2C irq handler
 *
 *  @param void
 *  @return - void
 */
void I2C0_IRQHandler(void)
{
	CORE_ATOMIC_IRQ_DISABLE();
	I2C_TransferReturn_TypeDef transfer_status=I2C_Transfer(I2C0);               //transfer status to check if i2c transaction is complete
	if (transfer_status==i2cTransferDone)                                        //if transfer done

	{
		if (bluetooth_i2c==write_state)
		{
			gecko_external_signal(event_write_bluetooth);						//write event
		}
		else if (bluetooth_i2c==read_state)
		{																		//read event
			gecko_external_signal(event_read_bluetooth);
		}
		else
		{
			gecko_external_signal(event_aqi_wr_done);							//aqi write done
		}
	}

	else if(transfer_status != i2cTransferDone)                                 //if transfer not complete
	{
		if (bluetooth_i2c==write_state)											//write event in progress
		{
			gecko_external_signal(event_i2c_progress_write);
		}
		else if (bluetooth_i2c==read_state)										//read event in progress
		{

			gecko_external_signal(event_i2c_progress_read);
		}
		else
		{
			gecko_external_signal(event_aqi_wr_progress);						//aqi write progress
		}

	}

	CORE_ATOMIC_IRQ_ENABLE();
}
