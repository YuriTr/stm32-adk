#include <string.h>

#include "iNEMO_util.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"	 
#include "semphr.h"
#include "serial.h"		  

/* Library includes. */
#include "stm32f10x_it.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_spi.h"

//#include "STLM75.h"
//#include "LSM303DLH.h"
//#include "LPS001DL_I2C.h"
#include "iNEMO_lib.h"


xComPortHandle uartHandle;

/* Turns iNEMO D3 led on */
void ledOn(void)
{
	GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
}

/* Turns iNEMO D3 led off */
void ledOff(void)
{
	GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_RESET);	
}


/* Innitializes UART. MUST be called after FreeRTOS scheduler has started */
int inemoUtilInit(void)
{
	uartHandle = xSerialPortInitMinimal(115200, 256);
	if(uartHandle != NULL){
		print("UART initialized succesfully\n\r");
		return 0;
	}

	return -1;
}


/* Prints on serial line a string */
int print(char* string)
{
	int len;

	len = strlen(string);
	if(uartHandle){
		vSerialPutString(uartHandle, (const signed char*)string, len);
		return len;
	}
	else{
		return -1;
	}
}


/* Prints on serial line hex value */
void printHex(unsigned int val)
{
	char string[5];
	u8 i;
	unsigned int rem;

	for(i=0; i<4;i++){
		rem = val % 16;
		if(rem >= 10)
			string[3-i] = 'A' + rem - 10;
		else
			string[3-i] = '0' + rem;
		val /= 16;
	}
	
	string[4] = 0;
	print(string);
}


/* This mimics the Arduino millis() call: returns the number of milliseconds
 * since system reset */
unsigned int millis(void)
{
	return xTaskGetTickCount()/portTICK_RATE_MS;
}

/* Arduino adapter: waits for  given amount of time (milliseconds) */
void delay(int delay)
{
	vTaskDelay(delay);
}


/* Call this when things go awfully wrong */
void panic(void)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET);
	for(;;);
}


/* Configure clocks, PLL, GPIOs clock gate */
void prvSetupHardware(void)
{
	/* Start with the clocks in their expected state. */
	RCC_DeInit();

	/* Enable HSE (high speed external clock). */
	RCC_HSEConfig( RCC_HSE_ON );

	/* Wait till HSE is ready. */
	while( RCC_GetFlagStatus( RCC_FLAG_HSERDY ) == RESET )
	{
	}

	/* 2 wait states required on the flash. */
	*( ( unsigned portLONG * ) 0x40022000 ) = 0x02;

	#ifdef STM32F10X_CL
	//PREDIV2 = 5, PLL2MUL=8, PREDIV1SRC=1, PREDIV1=5, PLL3MUL=8
	RCC->CFGR2 = (4<<4) | (6<<8) | (1<<16) | (4<<0) | (6<<12);
	//PLL2ON  PLL3ON
	RCC->CR |= (1<<26) | (1<<28);
	while(RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET)
	{
	}
	#endif
	
	/* HCLK = SYSCLK */
	RCC_HCLKConfig( RCC_SYSCLK_Div1 );

	/* PCLK2 = HCLK */
	RCC_PCLK2Config( RCC_HCLK_Div1 );

	/* PCLK1 = HCLK/2 */
#if BOARD_IS_INEMOV2
	RCC_PCLK1Config( RCC_HCLK_Div2 );	  //STMF103	Max 32MHz
#elif BOARD_IS_DISCOVERY
	RCC_PCLK1Config( RCC_HCLK_Div1 );	  //STMF100	Max 24MHz
#else
#error "Please define either BOARD_IS_INEMOV2 or BOARD_IS_DISCOVERY"
#endif
	
	/* PLLCLK = 8MHz * 9 = 72 MHz. */
#if BOARD_IS_INEMOV2
	RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_9 ); //STMF103 @72MHz
#elif BOARD_IS_DISCOVERY
	RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_3 );	 //STMF100 @24MHz
#else
#error "Please define either BOARD_IS_INEMOV2 or BOARD_IS_DISCOVERY"
#endif
  
	/* Enable PLL. */
	RCC_PLLCmd( ENABLE );

	/* Wait till PLL is ready. */
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
	{
	}

	/* Select PLL as system clock source. */
	RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );

	/* Wait till PLL is used as system clock source. */
	while( RCC_GetSYSCLKSource() != 0x08 )
	{
	}

	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE and AFIO clocks */
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
							| RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE );

	/* SPI2 Periph clock enable */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE );

	/* SPI1 Periph clock enable (for Max3421) */
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1, ENABLE );

	/* USART2 Periph clock enable (for debug on serial port) */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE );

	/* Set the Vector Table base address at 0x08000000 */
	NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );

	/* Enable prefetch */
	*((unsigned long*)(0x40022000)) = 0x12;

	iNEMO_HW_Config();
    iNEMO_Config();

}


/* Enables specific GPIOs for application */
void gpiosInit(void)
{
	GPIO_InitTypeDef gpioInit;

	/* Enable iNemo LED PC0 */
	GPIO_StructInit(&gpioInit);
	gpioInit.GPIO_Pin = GPIO_Pin_0;
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &gpioInit);

	/* Enable PB0 for output */
	GPIO_StructInit(&gpioInit);
	gpioInit.GPIO_Pin = GPIO_Pin_0;
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &gpioInit);

	/* Enable PC7 for input (Interrupt) */
	GPIO_StructInit(&gpioInit);
	gpioInit.GPIO_Pin = GPIO_Pin_7;
	gpioInit.GPIO_Mode = GPIO_Mode_AIN;
	gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &gpioInit);

	/* Enable PC3 for output (SPI Slave Select) */
	GPIO_StructInit(&gpioInit);
	gpioInit.GPIO_Pin = GPIO_Pin_3;
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &gpioInit);
}

