
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
//*****************************************************************************
//
//I2C GPIO chip address and resigster define
//
//*****************************************************************************
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
uint8_t 	I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
uint8_t 	I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr);
void		S800_I2C0_Init(void);
volatile uint8_t result;
uint32_t ui32SysClock;
uint8_t seg7[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x079,0x71,0x5c};
uint8_t ctw7[] = {0x7e,0xfc,0xf9,0xf3,0xe7,0xcf,0x9f,0x3f};
uint32_t t1[] =  {1,2,3,4,5,6,7,8};
uint32_t t2[] =  {8,1,2,3,4,5,6,7};
void Default(void);
uint32_t systick_digit_counter, systick_200ms_counter;
uint8_t systick_digit_state, systick_200ms_state;
int press_cnt,press_status,clock_cnt;
int latency[]={500,1000,2000,200};
int clock[100][60][4];
int timer[2];
int pressed[2];
void SysTickHandler(void)
{
	pressed[0]=GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0);
	pressed[1]=GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1);
	if (pressed[0] && pressed[1]){
		if (clock_cnt==1000){
			if (++timer[1]==60){
				timer[1]=0;
				if (++timer[0]==60) timer[0]=0;
			}
			clock_cnt=0;
		}
		else clock_cnt++;
	}
	
	if (systick_digit_counter==0) {
		systick_digit_counter=latency[press_cnt];
		systick_digit_state=1;
	}
	else systick_digit_counter--;
	
	if (systick_200ms_counter==0) {
		systick_200ms_counter=200;
		systick_200ms_state=1;
		if (!pressed[1]){
			if (++timer[1]==60){
				timer[1]=0;
				if (++timer[0]==60) timer[0]=0;
			}
		}
		if (!pressed[0]) {
			if (++timer[0]==60) timer[0]=timer[1]=0;
		}
	}
	else systick_200ms_counter--;
	if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)==0){
		if (press_status==0) press_status=1;
		systick_digit_state=0;
		systick_200ms_state=0;
		GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_0,GPIO_PIN_0);
	}
	else {
		if (press_status==1){
			press_status=0;
			press_cnt+=1;
			if (press_cnt==4) press_cnt=0;
		}
		GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_0,0x0);
	}
}

void myDelay(uint32_t value, int t)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){
			while ((result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<(t1[t]-1))));				//write port 2
			while ((result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[t1[t]])));						//write port 1
			Delay(30000);
			while ((result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<(t2[t]-1))));				//write port 2
			while ((result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[t2[t]])));						//write port 1
			Delay(30000);
	};
}

void Question2_3(void)
{
	int i, x;
	while (1)
	{
		x=1;
		for (i=0;i<8;i++)
		{
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,ctw7[i]);
			myDelay(50,i);
		}
	}
}

void Part2_3(void)
{
	int i, x;
	while (1)
	{
		x=1;
		for (i=0;i<8;i++)
		{
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[i+1]);						//write port 1
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,(uint8_t)(x));				//write port 2
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,ctw7[i]);
			x<<=1;
			while (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)==0);
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);												// Turn on the PF0 
			while (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)==0);
			Delay(800000);
			while (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)==0);
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);																// Turn off the PF0.
			while (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)==0);
			Delay(800000);	
			while (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)==0);
		}
	}
}

void S800_SYSTICK_INIT(void)
{
	SysTickPeriodSet(ui32SysClock/1000);
	SysTickEnable();
	SysTickIntEnable();
	IntMasterEnable();
	SysTickIntRegister(SysTickHandler);
	systick_digit_counter=latency[0];
	systick_200ms_counter=200;
	press_cnt=0;
	press_status=0;
	clock_cnt=0;
}


void Question3_4(void)
{
	int i=0, x=1;
	int pf0_status=0;
	while (1)
	{
		if (systick_200ms_state==1)
		{
			systick_200ms_state=0;
			if (pf0_status==1) {
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);// Turn on the PF0 
				pf0_status=0;
			}
			else{
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);
				pf0_status=1;
			}
		}
		if (systick_digit_state==1)
		{
			systick_digit_state=0;
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[i+1]);						//write port 1
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,(uint8_t)(x));				//write port 2
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,ctw7[i]);
			x<<=1;i++;
			if (i==8) { i=0;x=1; }
		}
	}
}


void ClockInit(void){
	int i,j;
	timer[0]=0;timer[1]=0;
	for (i=0;i<100;i++)
		for (j=0;j<60;j++){
			clock[i][j][0]=i/10;
			clock[i][j][1]=i%10;
			clock[i][j][2]=j/10;
			clock[i][j][3]=j%10;
		}
}


void Question5(void){
	int i;
	while (1){
		for (i=0;i<4;i++){
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[clock[timer[0]][timer[1]][i]]);
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,(uint8_t)(1<<i));				//write port 2
			Delay(35000);
		}
	}
}


int main(void)
{
	//use internal 16M oscillator, HSI
  ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 16000000);		
	
	S800_SYSTICK_INIT();
	S800_GPIO_Init();
	S800_I2C0_Init();
	ClockInit();
	//Default();
	//Part2_3();
	//Question2_3();
	//Question3_4();
	Question5();
}

void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}

void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);			//Set PF0 as Output pin
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}


void S800_I2C0_Init(void)
{
	uint8_t result;
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);//初始化i2c模块
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);//使用I2C模块0，引脚配置为I2C0SCL--PB2、I2C0SDA--PB3
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);//配置PB2为I2C0SCL
  GPIOPinConfigure(GPIO_PB3_I2C0SDA);//配置PB3为I2C0SDA
  GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);//I2C将GPIO_PIN_2用作SCL
  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);//I2C将GPIO_PIN_3用作SDA

	I2CMasterInitExpClk(I2C0_BASE,ui32SysClock, true);										//config I2C0 400k
	I2CMasterEnable(I2C0_BASE);	

	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT0,0x0ff);		//config port 0 as input
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT1,0x0);			//config port 1 as output
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT2,0x0);			//config port 2 as output 

	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_CONFIG,0x00);					//config port as output
	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0ff);				//turn off the LED1-8
	
}


uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData)
{
	uint8_t rop;
	while(I2CMasterBusy(I2C0_BASE)){};//如果I2C0模块忙，等待
		//
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
		//设置主机要放到总线上的从机地址。false表示主机写从机，true表示主机读从机
		
	I2CMasterDataPut(I2C0_BASE, RegAddr);//主机写设备寄存器地址
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);//执行重复写入操作
	while(I2CMasterBusy(I2C0_BASE)){};
		
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);//调试用

	I2CMasterDataPut(I2C0_BASE, WriteData);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);//执行重复写入操作并结束
	while(I2CMasterBusy(I2C0_BASE)){};

	rop = (uint8_t)I2CMasterErr(I2C0_BASE);//调试用

	return rop;//返回错误类型，无错返回0
}

uint8_t I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr)
{
	uint8_t value,rop;
	while(I2CMasterBusy(I2C0_BASE)){};	
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
//	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);		
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_SEND);//执行单词写入操作
	while(I2CMasterBusBusy(I2C0_BASE));
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	Delay(1);
	//receive data
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, true);//设置从机地址
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_RECEIVE);//执行单次读操作
	while(I2CMasterBusBusy(I2C0_BASE));
	value=I2CMasterDataGet(I2C0_BASE);//获取读取的数据
		Delay(1);
	return value;
}

void Default(void)
{
	while (1)
	{
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[0]);						//write port 1 				
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,(uint8_t)(1));					//write port 2
		
		result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0);	

		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);												// Turn on the PF0 
		Delay(800000);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);																// Turn off the PF0.
		Delay(800000);	
	}
}
