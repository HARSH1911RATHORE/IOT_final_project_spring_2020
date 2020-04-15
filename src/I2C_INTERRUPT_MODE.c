#include "I2C_INTERRUPT_MODE.h"
#include "i2c.h"
#include "state_machine.h"

uint8_t write_data=0xF3;													//setting no hold master mode
I2C_TransferSeq_TypeDef seq;												//transfer seq structure for read and write i2c
I2C_TransferSeq_TypeDef seq1;
uint32_t one_byte=1;                                                        //one byte of write data
uint8_t read_data_int[2]={0};                                               //2 bytes of read buffer

bool event_i2c=false;														//checking for event_i2c interrupt event

uint32_t i2c_evt;															//checking the type of i2c interrupt event occured

int bluetooth_i2c=0;


/**
 * I2C transfer function to write data to slave address
 *
 * @param void
 *
 */
void I2C_transfer_interrupt_init()
{
	seq.addr=slave_address<<1;                                              //shifting the slave address by one
	seq.flags=I2C_FLAG_WRITE;                                               //i2c write mode
	seq.buf[0].data=&write_data;                                            //selecting no hold master mode
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
 * I2C read function to read data to slave address
 *
 * @param void
 *
 */

void I2C_read_interrupt()
{

	seq.addr=slave_address<<1;                                                   //shifting the slave address by one
	seq.flags=I2C_FLAG_READ;                                                     //i2c read mode
	seq.buf[0].data=read_data_int;                                               //defining read buffer where temperature will be read
	seq.buf[0].len=2;                                                            //writing two bytes of data
	I2C_TransferReturn_TypeDef check_read_int = I2C_TransferInit(I2C0,&seq);

	if(check_read_int != i2cTransferDone)                                		 //if transfer not done print transfer in progress
	{
		LOG_ERROR("Transfer in progress");
	}
}

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


	}

	CORE_ATOMIC_IRQ_ENABLE();
}
