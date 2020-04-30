/*
 * gpio.c
 *
 *  Created on: Dec 12, 2018
 *      Author: Dan Walkes
 */
#include "gpio.h"
#include "em_gpio.h"                 //calling gpio specific header files
#include <string.h>
#include "log.h"
#include "em_core.h"
#include "native_gecko.h"
#define PB0_port gpioPortF			//pb0 button port
#define PB0_pin (6)					//pb0 pin
#define PB0 (64)					//pb0 interrupt number

extern int button;

/**
 * Gpio init setting for led 0 and 1
 *
 * @param none
 * @return- void
 *
 */
void gpioInit()
{
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);      //Drive strength for gpio led0

	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);
	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);      //Drive strength for gpio led 1

	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);
}

/**
 * Gpio external interrupt init enable
 *
 * @param none
 * @return- void
 *
 */
void gpio_interrupt_enable()
{

	GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeInputPull, 1);		//set pin mode and pull up for pb0

	GPIO_ExtIntConfig(PB0_port,PB0_pin,4,1,1,1);					//configure interrupt for pb0

	GPIO_IntEnable(_GPIO_IEN_RESETVALUE);							//set interrupt enable flag
	NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);							//clear nvic for even gpio handle
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);									//set nvic for even gpio handle

}

/**
 * Gpio external interrupt irq handler
 *
 * @param void
 * @return- void
 *
 */

void GPIO_EVEN_IRQHandler(void)
{
	LOG_INFO("BUTTON PB0 has been pressed\n");
	CORE_ATOMIC_IRQ_DISABLE();
	uint32_t status = GPIO_IntGet();
	button|=PB0_PRESSED;						//button pressed
	gecko_external_signal(button);   //external signal bluetooth event
	GPIO_IntClear(status);
	CORE_ATOMIC_IRQ_ENABLE();
}


void gpioLed0SetOn()                                             //enabling the led 0
{
	GPIO_PinOutSet(LED0_port,LED0_pin);
}
void gpioLed0SetOff()                                            //disabling the led 0
{
	GPIO_PinOutClear(LED0_port,LED0_pin);
}
void gpioLed1SetOn()											 //enabling the led 1
{
	GPIO_PinOutSet(LED1_port,LED1_pin);
}
void gpioLed1SetOff()											 //disabling the led 1
{
	GPIO_PinOutClear(LED1_port,LED1_pin);
}
void gpioLed0Toggle()
{
	GPIO_PinOutToggle(LED0_port,LED0_pin);
}
void gpioSetDisplayExtcomin(bool high)							//display lcd extcomin pin
{
	if(high == true)
	{
		EXTCOMIN_SET;											//if display on pin set
	}
	else
	{
		EXTCOMIN_CLEAR;											//else cleared
	}
}
void gpioEnableDisplay()
{
	ENABLE_DISPLAY;
}
