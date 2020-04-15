/*
 * log.c
 *
 *  Created on: Dec 18, 2018
 *      Author: Dan Walkes
 */

#include "retargetserial.h"
#include "log.h"
#include <stdbool.h>
#include "letimer.h"
uint32_t rollover=0;
uint32_t freq=0;
#if INCLUDE_LOGGING
/**
 * @return a timestamp value for the logger, typically based on a free running timer.
 * This will be printed at the beginning of each log message.
 */
uint32_t loggerGetTimestamp(void)
{

	uint32_t present_value;                                                             //variable to check the present value of tick
	present_value=LETIMER_CounterGet(LETIMER0);
	uint32_t period=LETIMER_CompareGet(LETIMER0,0);                                     //period variable to check the comp0 value
	uint32_t timerGetRunTimeMilliseconds=((period-present_value)+(rollover*3*freq))*1000/freq;   //value in mili seconds since the time i2c starts
	return timerGetRunTimeMilliseconds;                                                 //timer in mili is max value - present value + rollover when uf occurs

}

/**
 * Initialize logging for Blue Gecko.
 * See https://www.silabs.com/community/wireless/bluetooth/forum.topic.html/how_to_do_uart_loggi-ByI
 */
void logInit(void)
{
	RETARGET_SerialInit();
	/**
	 * See https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__RetargetIo.html#ga9e36c68713259dd181ef349430ba0096
	 * RETARGET_SerialCrLf() ensures each linefeed also includes carriage return.  Without it, the first character is shifted in TeraTerm
	 */
	RETARGET_SerialCrLf(true);
	LOG_INFO("Initialized Logging");
}

/**
 * Block for chars to be flushed out of the serial port.  Important to do this before entering SLEEP() or you may see garbage chars output.
 */
void logFlush(void)
{
	RETARGET_SerialFlush();
}
#endif
