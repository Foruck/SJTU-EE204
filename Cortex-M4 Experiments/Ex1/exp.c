#define PART_TM4C1294NCPDT
#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_types.h"
#include "pin_map.h"
#include "sysctl.h"


#define   FASTFLASHTIME			(uint32_t)500000
#define   SLOWFLASHTIME			(uint32_t)4000000

uint32_t delay_time,key_value;
uint32_t cnt;
uint32_t tinyc;

void 		Delay1(uint32_t value);
void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);
void		Default(void);
void		T3(void)
{
	while(1)
  {
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	== 0)						//USR_SW1-PJ0 pressed
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);			// Turn on the LED_M0.
		else
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED_M0.
		
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)	== 0)						//USR_SW1-PJ1 pressed
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);			// Turn on the LED_M1.
		else
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);							// Turn off the LED_M1.
   }
}

void T4(void)
{
	tinyc=0;
	cnt=0;
	delay_time=FASTFLASHTIME;
	while(1)
  {
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	== 0 && tinyc == 0)						//USR_SW1-PJ0 pressed
			tinyc=1;
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	== 1 && tinyc == 1)
		{
			tinyc=0;
			cnt++;
		}
		if (cnt==1)
		{
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);			// Turn on the LED.
			Delay1(delay_time);
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED.
			Delay1(delay_time);
		}
		else if (cnt==2)
		{
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED.
		}
		else if (cnt==3)
		{
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);			// Turn on the LED.
			Delay1(delay_time);
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);							// Turn off the LED.
			Delay1(delay_time);
		}
		else if (cnt==4)
		{
			cnt=0;
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);							// Turn off the LED.
		}
	}
}
int main(void)
{
	SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_OSC),16000000);
	//SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_OSC), 5000000); 
	//use MOSC as the system clock of 5MHz
	//SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 40000000);
	//use PLL to generate 480MHz, divide the PLL frequency to 40 MHz
	S800_GPIO_Init();
	//Default();
	//T3();
	T4();
}

void Delay1(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++)
	{
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	== 0 && tinyc == 0)						//USR_SW1-PJ0 pressed
			tinyc=1;
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	== 1 && tinyc == 1)
		{
			tinyc=0;
			cnt++;
			break;
		}
	}
}

void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){}
}


void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1);			//Set PF0 as Output pin
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}

void Default(void)
{
	while(1)
  {
		key_value = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	;				//read the PJ0 key value

		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	== 0)						//USR_SW1-PJ0 pressed
			delay_time							= FASTFLASHTIME;
		else
			delay_time							= SLOWFLASHTIME;
		
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);			// Turn on the LED.
		Delay(delay_time);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED.
		Delay(delay_time);
   }
}
