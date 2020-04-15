#include "letimer.h"
#include "state_machine.h"
#include "display.h"
const SLEEP_EnergyMode_t sleep_mode=sleepEM2;   // setting the mode at which the system will operate

extern uint32_t freq;
extern uint32_t rollover;                     //rollover value incremented when underflow interrupt triggered

uint32_t letimer_evt=0;

int bluetooth=0;

/**
 * Letimer clock initilisation function, selecting clock
 * frequency as per the mode the system works on
 * @param void
 * 32768 Hz Low Frequency Crystal Oscillator (LFXO)
 * 1000 Hz Ultra Low Frequency RC Oscillator (ULFRCO)
 * Prescalers on each clock tree to optimize energy
 *
 */
void clockInit()
{

	if(sleep_mode<3 && sleep_mode >=0)                          //For sleep mode from 0 to 2 , LFXO clock is used
	{
		CMU_OscillatorEnable(cmuOsc_LFXO,true,true);			//Low frequency crystal oscillator enabled
		CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO); 		//Selecting branch as LFA and  LFXO as the clock source
		period=period_set;
		freq=8192;                                              //setting comp0 and freq values

	}
	else if(sleep_mode==3)
	{
		CMU_OscillatorEnable(cmuOsc_ULFRCO,true,true);			// ULtra Low frequency crystal oscillator enabled
		CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO); 	//Selecting branch as LFA and  LFXO as the clock sour
		period=period_set_deep_sleep;
		freq=1000;                                              //setting comp0 and freq values
	}

   if (sleep_mode!=3)
   {
	CMU_ClockDivSet(cmuClock_LETIMER0,4);                      //if sleep mode is not EM3, prescale the clock by 4
   }

	CMU_ClockEnable(cmuClock_LFA, true);                       //clock enable

	CMU_ClockEnable(cmuClock_LETIMER0,true);                   //letimer clock enable

}

/**
 * Letimer interrupt handler which triggers depending on the count value set in the comp0 register
 * which is chosen as top
 * @param -void
 * return- nothing
 */

void LETIMER0_IRQHandler(void)
{
	    CORE_ATOMIC_IRQ_DISABLE();
		uint32_t flags =LETIMER_IntGetEnabled(LETIMER0);                //checking the interrupt flag
		LETIMER_IntClear(LETIMER0,flags);                               //clear the interrupt
		if((flags & LETIMER_IF_COMP1) == LETIMER_IF_COMP1)                  //if underflow interrupt has occurred, then set led on
		{

			if (bluetooth==timer_80)
			{
				gecko_external_signal(event_timer_80_bluetooth);		//check for 80 milisecond event and send the signal to bluetooth stack
			}
			else if (bluetooth==timer_10)
			{
				gecko_external_signal(event_timer_10_bluetooth);		//check for 10 milisecond event and send the signal to bluetooth stack
			}
			else if (bluetooth==timer_5)
			{
				gecko_external_signal(event_timer_5_bluetooth);			//check for 5 milisecond event and send the signal to bluetooth stack
			}


			LETIMER_CompareSet(LETIMER0,1,0xFFFF);						//set value to max
			LETIMER_IntDisable(LETIMER0,LETIMER_IFC_COMP1);

		}
		if((flags & LETIMER_IF_UF) == LETIMER_IF_UF)                  //if underflow interrupt has occurred, then set led on
		{
			rollover++;												//for logger timestamp
			displayUpdate();
			gecko_external_signal(event_bluetooth);					//check for 3 second uf interrupt and send the signal to bluetooth stack

		}

		CORE_ATOMIC_IRQ_ENABLE();
}

/**
 * Timerwaitus function which sets the required wait duration by making use of letimer count value
 *
 * @param us_wait- the value in microseconds which is required for waiting
 * @return- void
 *
 */

void timerWaitUs(int us_wait)                                                 //to calculate timer wait in micro seconds
{
	uint32_t present_value;                                                   //variable to check the present value of tick
	uint32_t us_ticks;                                                        //ticks required
	us_ticks=freq * us_wait;                                                  //us ticks is calculated by multiplying present freq with us wait
	us_ticks=us_ticks/1000000;	                                              //for micro seconds the value of seconds will be divided by 10^6
	present_value=LETIMER_CounterGet(LETIMER0);                               //checking the present value of ticks
	if (present_value>us_ticks)											      //checking if present value is greater than value of ticks required
	{
		uint32_t count_value1=present_value-us_ticks;                         //if yes calculate the time letimer will have to count to reach the required ticks
		while(LETIMER_CounterGet(LETIMER0)!=count_value1);
	}
	else
	{                                                                         //eg. 4000 required, and present count is 1000, and comp0 is 10000
		uint32_t count_value=us_ticks-present_value;                          //count value=4000-1000=3000
		uint32_t subtract_value=period-count_value;                           //subtract value=comp0  value - count= 10000-3000=7000
		while(LETIMER_CounterGet(LETIMER0)!=subtract_value);
	}
}


/**
 * timerSetEventInMs function which sets the required wait duration by making use of letimer interrupt
 *
 * @param ms_wait- the value in miliseconds which is required for waiting
 * @return- void
 *
 */
void timerSetEventInMs(int ms_until_wait)
{
	uint32_t present_value;                                                   //variable to check the present value of tick
	uint32_t ms_until_wait_ticks;                                             //ticks required
	ms_until_wait_ticks=freq * ms_until_wait;                                 //us ticks is calculated by multiplying present freq with us wait
	ms_until_wait_ticks=ms_until_wait_ticks/1000;	                          //for mili seconds the value of seconds will be divided by 1000
	present_value=LETIMER_CounterGet(LETIMER0);                               //checking the present value of ticks
	if (present_value>ms_until_wait_ticks)				                      //checking if present value is greater than value of ticks required
	{
		uint32_t count_value1=present_value-ms_until_wait_ticks;             //if yes calculate the time letimer will have to count to reach the required ticks
		LETIMER_CompareSet(LETIMER0,1,count_value1);					     //new value of comp1 set
		LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);

	}
	else
	{                                                                         //eg. 4000 required, and present count is 1000, and comp0 is 10000
		uint32_t count_value=ms_until_wait_ticks-present_value;               //count value=4000-1000=3000
		uint32_t subtract_value=period-count_value;                           //subtract value=comp0  value - count= 10000-3000=7000

		LETIMER_CompareSet(LETIMER0,1,subtract_value);

		LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);
	}
}
/**
 * Letimer init default values
 *
 *
 */
const LETIMER_Init_TypeDef letimer=
 {
 		.enable=false,
 		.debugRun=false,                //initializing the default values for LETIMER and setting compo as top and disabling letimer initially
 		.comp0Top=true,
 		.bufTop=false,
 		.out0Pol=0,
 		.out1Pol=0,
		.ufoa0= letimerUFOANone,
		.ufoa1= letimerUFOANone,
 		.repMode= letimerRepeatFree,
		.topValue=0
  };


/**
 * Letimer init function for setting count value, enabling interrupt and nvic interrupt enable
 *
 * @param void
 * @return- void
 *
 */
void letimerInit()
{

	LETIMER_Init(LETIMER0,&letimer);                   //Initializing letimer
	LETIMER_CompareSet(LETIMER0,0,period);             //setting comp0 register as period

	LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF );      //Enable Underflow interrupt
	NVIC_EnableIRQ(LETIMER0_IRQn);                     //Enabling the letimer interrupt in Nested vector interrupt table

	LETIMER_Enable(LETIMER0, true);                   //LETIMER0 enabled

}
