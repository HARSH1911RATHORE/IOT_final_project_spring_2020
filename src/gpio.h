/*
 * gpio.h
 *
 *  Created on: Dec 12, 2018
 *      Author: Dan Walkes
 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_
#include <stdbool.h>

#define	LED0_port gpioPortF
#define LED0_pin 4                            //led pin and port set
#define LED1_port gpioPortF
#define LED1_pin 5

#define GPIO_SET_DISPLAY_EXT_COMIN_IMPLEMENTED 	(1)       //extcomin pin display pin implemented

#define GPIO_DISPLAY_SUPPORT_IMPLEMENTED		(1)		//display support implemented

#define ENABLE_DISPLAY            GPIO_PinOutSet(gpioPortD, 15)		//enable display
#define EXTCOMIN_SET 			  GPIO_PinOutSet(gpioPortD, 13)		//extcomin display pin enable
#define EXTCOMIN_CLEAR  		  GPIO_PinOutClear(gpioPortD, 13)	//extcomin display pin clear

#define	PB0_PRESSED	1<<9				//pb0 button press
#define	PB0_RELEASED 1<<10				//pb0 button release

/**
 * @BRIEF  initiates gpio
 *
 * given- none
 * @param -none
 * return- nothing
 */

void gpioInit();
/**
 * @BRIEF  sets led 0 gpio pin to switch it on
 *
 * given- none
 * @param -none
 * return- nothing
 */

void gpioLed0SetOn();                         //prototype declarations
/**
 * @BRIEF  sets led 0 gpio pin to switch it off
 *
 * given- none
 * @param -none
 * return- nothing
 */

void gpioLed0SetOff();
/**
 * @BRIEF  sets led 1 gpio pin to switch it on
 *
 * given- none
 * @param -none
 * return- nothing
 */

void gpioLed1SetOn();
/**
 * @BRIEF  sets led 1 gpio pin to switch it off
 *
 * given- none
 * @param -none
 * return- nothing
 */

void gpioLed1SetOff();
/**
 * @BRIEF  sets led 0 gpio pin to toggle it
 *
 * given- none
 * @param -none
 * return- nothing
 */

void gpioLed0Toggle();

/**
 * @BRIEF  sets led 0 gpio pin to toggle it
 *
 * given- none
 * @param -none
 * return- nothing
 */
void gpioSetDisplayExtcomin(bool high);

/**
 * @BRIEF  sets display pin
 *
 * given- none
 * @param -none
 * return- nothing
 */
void gpioEnableDisplay();

/**
 * @BRIEF  enables interrupt for external gpio interrupt
 *
 * given- none
 * @param -none
 * return- nothing
 */
void gpio_interrupt_enable();

/**
 * @BRIEF  even external gpio handler for interrupt
 *
 * given- none
 * @param -void
 * return- nothing
 */
void GPIO_EVEN_IRQHandler(void);


#endif /* SRC_GPIO_H_ */
