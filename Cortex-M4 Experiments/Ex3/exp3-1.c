#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_i2c.h"
#include "hw_types.h"
#include "i2c.h"
#include "pin_map.h"
#include "sysctl.h"
#include "systick.h"
#include "interrupt.h"
#include "uart.h"
#include "hw_ints.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#define SYSTICK_FREQUENCY		1000			//1000hz
#define	I2C_FLASHTIME				500				//500mS
#define GPIO_FLASHTIME			300				//300mS
#define TCA6424_I2CADDR 					0x22
#define PCA9557_I2CADDR						0x18
#define PCA9557_INPUT							0x00
#define	PCA9557_OUTPUT						0x01
#define PCA9557_POLINVERT					0x02
#define PCA9557_CONFIG						0x03
#define TCA6424_CONFIG_PORT0			0x0c
#define TCA6424_CONFIG_PORT1			0x0d
#define TCA6424_CONFIG_PORT2			0x0e
#define TCA6424_INPUT_PORT0				0x00
#define TCA6424_INPUT_PORT1				0x01
#define TCA6424_INPUT_PORT2				0x02
#define TCA6424_OUTPUT_PORT0			0x04
#define TCA6424_OUTPUT_PORT1			0x05
#define TCA6424_OUTPUT_PORT2			0x06
void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);
void		S800_I2C0_Init(void);
void 		S800_UART_Init(void);
uint8_t 	I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
uint8_t 	I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr);
volatile uint16_t systick_10ms_couter,systick_100ms_couter,systick_200ms_couter,systick_500ms_couter;
volatile uint8_t	systick_10ms_status,systick_100ms_status,systick_200ms_status,systick_500ms_status;
volatile uint8_t result,cnt,key_value,gpio_status;
volatile uint8_t rightshift = 0x01;
uint32_t ui32SysClock, ui32IntPrioritySystick, ui32IntPriorityUart0;
uint32_t ui32IntPriorityGroup, ui32IntPriorityMask;
uint8_t seg7[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x079,0x71,0x5c};
uint8_t uart_receive_char;
uint32_t sec,min,hour,time_cnt,month;
char months[]="JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC";
char characters[]="A\0B\0C\0D\0";
int e,f;
int tmpstatus=0,delta=1;;

void UARTStringPutNonBlocking(const char *cMessage){
	while(*cMessage!='\0'){
		while (!UARTCharPutNonBlocking(UART0_BASE,*cMessage));
		cMessage++;
	}
}
bool checktime(char inst[]){
	char tmp[30]="GETTIME";
	uint32_t ds,dm,dh;
	if (!strcmp(inst,tmp)) return true;
	strcpy(tmp,inst);tmp[5]='\0';tmp[8]='\0';tmp[11]='\0';
	if (inst[0]=='S' && inst[1]=='E' && inst[2]=='T'){
		hour=atoi(&tmp[3]);
		min=atoi(&tmp[6]);
		sec=atoi(&tmp[9]);
		return true;
	}
	else if (inst[0]=='I' && inst[1]=='N' && inst[2]=='C'){
		dh=atoi(&tmp[3]);
		dm=atoi(&tmp[6]);
		ds=atoi(&tmp[9]);
		sec+=ds;
		min+=dm+sec/60;
		sec%=60;
		hour+=dh+min/60;
		min%=60;
		if (hour>=100) hour=sec=min=0;
		return true;
	}
	return false;
}

int checkmonth(char inst[]){
	int len,x,y;
	len=strlen(inst);
	if (len!=6) return -1;
	inst[3]='\0';
	x=(strstr(months,inst)-months);
	if (x%3!=0) return -1;
	y=atoi(&inst[4]);
	y+=x/3;
	y%=12;
	return y*3;
}
bool checkplus(char inst[]){
	int a,b,c,d;
	if (strlen(inst)!=11) return false;
	inst[2]='\0';
	inst[5]='\0';
	inst[8]='\0';
	a=atoi(inst);
	b=atoi(&inst[3]);
	c=atoi(&inst[6]);
	d=atoi(&inst[9]);
	if (!a && !b && !c && !d) return false;
	f=b+d;
	e=(a+c+f/60)%60;
	f%=60;
	return true;
}
void Default(void){
	volatile uint16_t	i2c_flash_cnt,gpio_flash_cnt;
	while (1)
	{
		if (systick_10ms_status)
		{
			systick_10ms_status		= 0;
			if (++gpio_flash_cnt	>= GPIO_FLASHTIME/10)
			{
				gpio_flash_cnt			= 0;
				if (gpio_status)
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,GPIO_PIN_0 );
				else
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,0);
				gpio_status					= !gpio_status;
			
			}
		}
		if (systick_100ms_status)
		{
			systick_100ms_status	= 0;
			if (++i2c_flash_cnt		>= I2C_FLASHTIME/100)
			{
				i2c_flash_cnt				= 0;
				result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[cnt+1]);	//write port 1 				
				result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);	//write port 2
		
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);	

				cnt++;
				rightshift= rightshift<<1;

				if (cnt		  >= 0x8)
				{
					rightshift= 0x01;
					cnt 			= 0;
				}

			}
		}
	}
}

void GetUpperC(char inst[]){
	char *ptr;
	ptr=inst;
	while (*ptr!='\0'){
		if (*ptr>='a' && *ptr<='z') *ptr=*ptr-'a'+'A';
		ptr++;
	}
}
void UART0_Handler(void){
	char inst[200]="";
	int len=0,x;
	char mode1[]="ATCLASS";
	char mode2[]="ATSTUDENTCODE";
	char clas[]="CLASS03302";
	char stdcode[]="CODE090259";
	char buffer[30];
	int32_t uart0_int_status;
	uart0_int_status=UARTIntStatus(UART0_BASE,true);
	UARTIntClear(UART0_BASE,uart0_int_status);
	while (UARTCharsAvail(UART0_BASE)){
		inst[len++]=UARTCharGetNonBlocking(UART0_BASE);
		GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_1,GPIO_PIN_1);
		Delay(1000);
	}
	GetUpperC(inst);
	/*
	UARTCharPutNonBlocking(UART0_BASE,'\n');
	UARTStringPutNonBlocking(inst);
	UARTCharPutNonBlocking(UART0_BASE,'\n');
	*///echo
	if (!strcasecmp(inst,&characters[0])) delta=-1;
	else if (!strcasecmp(inst,&characters[2])) delta=0;
	else if (!strcasecmp(inst,&characters[4])) delta=1;
	else if (!strcasecmp(inst,&characters[6])) {
		delta=1;sec=0;hour=0;min=0;
	}
	/*
	if (!strcasecmp(inst,mode1)) UARTStringPutNonBlocking(clas);
	else if (!strcasecmp(inst,mode2)) UARTStringPutNonBlocking(stdcode);
	else if (checktime(inst)){
		sprintf(buffer,"TIME%02d:%02d:%02d",hour,min,sec);
		UARTStringPutNonBlocking(buffer);
		Delay(20000);
	}
	else if ((x=checkmonth(inst))>0){
		UARTCharPutNonBlocking(UART0_BASE,months[x]);
		UARTCharPutNonBlocking(UART0_BASE,months[x+1]);
		UARTCharPutNonBlocking(UART0_BASE,months[x+2]);
	}
	else if (checkplus(inst)){
		sprintf(buffer,"%02d:%02d",e,f);
		UARTStringPutNonBlocking(buffer);
	}
	*/
	/*
	*/
	while (!GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)){
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[hour/10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);	//write port 2
		Delay(30000);
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[hour%10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<1);	//write port 2
		Delay(30000);
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[min/10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<3);	//write port 2
		Delay(30000);
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[min%10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<4);	//write port 2
		Delay(30000);
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[sec/10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<6);	//write port 2
		Delay(30000);
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[sec%10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<7);	//write port 2
		Delay(30000);
	}
	GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_1,0);
}
void DisplayID(void){
	while (1){
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[0]);//write port 1 				
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<2);	//write port 2
			Delay(30000);
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[2]);//write port 1 				
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<3);	//write port 2
			Delay(30000);
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[5]);//write port 1 				
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<4);	//write port 2
			Delay(30000);
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[9]);//write port 1 				
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<5);	//write port 2
			Delay(30000);
			if (!GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) || tmpstatus) return;
	}
}
void SysTick_UART_init(void){
	time_cnt=1000;
  SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY);
	SysTickEnable();
	SysTickIntEnable();
	IntEnable(INT_UART0);
	UARTIntEnable(UART0_BASE,UART_INT_RX | UART_INT_RT);
  IntMasterEnable();
	ui32IntPriorityMask=IntPriorityMaskGet();
	IntPriorityGroupingSet(3);
	IntPrioritySet(INT_UART0,0x0e0);
	IntPrioritySet(FAULT_SYSTICK,3);
	ui32IntPriorityGroup=IntPriorityGroupingGet();
	ui32IntPriorityUart0=IntPriorityGet(INT_UART0);
	ui32IntPrioritySystick=IntPriorityGet(FAULT_SYSTICK);
}

void Clock(void){
	int t=0;
	while (1){
		if (systick_500ms_status){
			systick_500ms_status=0;
			if (delta!=0){
				t=~t;
				if (t) result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xf7);
				else result=result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xff);
			}
		}
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[hour/10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);	//write port 2
		Delay(30000);
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[hour%10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<1);	//write port 2
		Delay(30000);
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[min/10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<3);	//write port 2
		Delay(30000);
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[min%10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<4);	//write port 2
		Delay(30000);
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[sec/10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<6);	//write port 2
		Delay(30000);
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[sec%10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<7);	//write port 2
		Delay(30000);
	}
}
void basic_runninghorse(void){
	cnt=0;rightshift=1;
	while (1){
		if (systick_200ms_status){
			systick_200ms_status=0;
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);
			cnt+=1;
			rightshift<<=1;
			if (cnt>=8) {cnt=0;rightshift=1;}
		}
	}
}
void task1(void){
	int t=0;
	int cnt=0,rightshift=1;
	int sw2_status=0,sw2_cnt=0;
	while (1){
		if (!GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)){
			GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,GPIO_PIN_1);
		}
		else GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,0);
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)){
			if (systick_500ms_status){
				systick_500ms_status=0;
				t=~t;
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,t);
			}
		}
		else if (systick_100ms_status){
			systick_100ms_status=0;
			t=~t;
			GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,t);
		}
		if (!GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)){
			sw2_status=1;
		}
		if (sw2_status==1 && GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)){
			sw2_status=0;
			sw2_cnt=(sw2_cnt+1)%6;
			if (sw2_cnt!=4 && sw2_cnt!=5) {
				cnt=0;rightshift=1;
				if (sw2_cnt==2) rightshift=2;
			}
		}
		if (sw2_cnt==0){
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xFF);
		}
		if (sw2_cnt==1){
			if (systick_200ms_status){
				systick_200ms_status=0;
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);
				cnt+=2;
				rightshift<<=2;
				if (cnt>=8) {cnt=0;rightshift=1;}
			}
		}
		if (sw2_cnt==2 && systick_200ms_status){
			systick_200ms_status=0;
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);
			cnt+=2;
			rightshift<<=2;
			if (cnt>=8) {cnt=1;rightshift=2;}
		}
		if (sw2_cnt==3 && systick_200ms_status){
			systick_200ms_status=0;
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);
			cnt+=1;
			rightshift<<=1;
			if (cnt>=8) {cnt=0;rightshift=1;}
		}
		if (sw2_cnt==4 && systick_200ms_status){
			systick_200ms_status=0;
		}
		if (sw2_cnt==5 && systick_200ms_status){
			systick_200ms_status=0;
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~rightshift);
			cnt+=1;
			rightshift<<=1;
			if (cnt>=8) {cnt=0;rightshift=1;}
		}
	}
}
void SysTick_Handler(void){
	if (systick_100ms_couter	!= 0)
		systick_100ms_couter--;
	else
	{
		systick_100ms_couter	= SYSTICK_FREQUENCY/10;
		systick_100ms_status 	= 1;
	}
	if (systick_200ms_couter	!= 0)
		systick_200ms_couter--;
	else
	{
		systick_200ms_couter	= 200;
		systick_200ms_status 	= 1;
	}
	
	if (systick_500ms_couter	!= 0)
		systick_500ms_couter--;
	else
	{
		systick_500ms_couter	= 500;
		systick_500ms_status 	= 1;
	}
	
	if (systick_10ms_couter	!= 0)
		systick_10ms_couter--;
	else
	{
		systick_10ms_couter		= SYSTICK_FREQUENCY/100;
		systick_10ms_status 	= 1;
	}
	if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0)
	{
		tmpstatus=1;
		systick_100ms_status	= systick_10ms_status = 0;
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,GPIO_PIN_0);		
		while (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0){
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[hour/10]);//write port 1 				
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);	//write port 2
			Delay(30000);
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[hour%10]);//write port 1 				
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<1);	//write port 2
			Delay(30000);
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[min/10]);//write port 1 				
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<3);	//write port 2
			Delay(30000);
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[min%10]);//write port 1 				
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<4);	//write port 2
			Delay(30000);
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[sec/10]);//write port 1 				
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<6);	//write port 2
			Delay(30000);
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[sec%10]);//write port 1 				
			result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<7);	//write port 2
			Delay(30000);
		}
	}
	else
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,0);
	if (time_cnt!=0) time_cnt--;
	else {
		time_cnt=1000;
		sec+=delta;
		if (delta==1){
			if (sec==60){
				sec=0;
				if (++min==60){
					min=0;
					if (++hour==100) hour=0;
				}
			}
		}
		else {
			if (sec==-1){
				sec=59;
				if (--min==-1){
					min=59;
					if (--hour==-1) hour=99;
				}
			}
		}
	}
}

void dividedDisplay(void){
	int cnt=0;
	int sw2_status,sw2_cnt;
	while (1){
		if (!GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)){
			sw2_status=1;
		}
		if (sw2_status==1 && GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)){
			sw2_status=0;
			sw2_cnt=(sw2_cnt+1)%6;
			if (sw2_cnt!=4 && sw2_cnt!=5) {
				cnt=0;rightshift=1;
				if (sw2_cnt==2) rightshift=2;
			}
		}
		if (systick_100ms_status){
			systick_100ms_status=0;
			cnt++;
			if (cnt==10) cnt=0;
		}
		result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~(1<<cnt));
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[sec/10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);	//write port 2
		Delay(30000);
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[sec%10]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,2);	//write port 2
		Delay(30000);
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[cnt]);//write port 1 				
		result=I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,4);	//write port 2
		Delay(30000);
	}
}
int main(void){
	//use internal 16M oscillator, PIOSC
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 16000000);		
	//ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 8000000);		
	//use external 25M oscillator, MOSC
  //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_OSC), 25000000);		
	//use external 25M oscillator and PLL to 120M
  //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 120000000);;		
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 20000000);
	S800_GPIO_Init();
	S800_I2C0_Init();
	S800_UART_Init();
	SysTick_UART_init();
	//basic_runninghorse();
	//task1();
	//Default();
	//DisplayID();
	//Clock();
	dividedDisplay();
}

//unchanged
void Delay(uint32_t value){
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}


void UARTStringPut(const char *cMessage){
	while(*cMessage!='\0')
		UARTCharPut(UART0_BASE,*(cMessage++));
}
void S800_UART_Init(void){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);						//Enable PortA
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));			//Wait for the GPIO moduleA ready

	GPIOPinConfigure(GPIO_PA0_U0RX);												// Set GPIO A0 and A1 as UART pins.
  GPIOPinConfigure(GPIO_PA1_U0TX);    			

  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(UART0_BASE, ui32SysClock,115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));
	UARTFIFOLevelSet(UART0_BASE,UART_FIFO_TX7_8,UART_FIFO_RX7_8);
	UARTStringPut((uint8_t *)"Hello, world!");
}
void S800_GPIO_Init(void){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);						//Enable PortN	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));			//Wait for the GPIO moduleN ready		
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1);			//Set PF0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);			//Set PN0 as Output pin

	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}

void S800_I2C0_Init(void){
	uint8_t result;
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);
  GPIOPinConfigure(GPIO_PB3_I2C0SDA);
  GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

	I2CMasterInitExpClk(I2C0_BASE,ui32SysClock, true);										//config I2C0 400k
	I2CMasterEnable(I2C0_BASE);	

	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT0,0x0ff);		//config port 0 as input
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT1,0x0);			//config port 1 as output
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT2,0x0);			//config port 2 as output 

	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_CONFIG,0x00);					//config port as output
	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0ff);				//turn off the LED1-8
	
}


uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData){
	uint8_t rop;
	while(I2CMasterBusy(I2C0_BASE)){};
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C0_BASE)){};
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);

	I2CMasterDataPut(I2C0_BASE, WriteData);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(I2C0_BASE)){};

	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	return rop;
}

uint8_t I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr){
	uint8_t value,rop;
	while(I2CMasterBusy(I2C0_BASE)){};	
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
//	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);		
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_SEND);
	while(I2CMasterBusBusy(I2C0_BASE));
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	Delay(1);
	//receive data
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, true);
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_RECEIVE);
	while(I2CMasterBusBusy(I2C0_BASE));
	value=I2CMasterDataGet(I2C0_BASE);
		Delay(1);
	return value;
}


