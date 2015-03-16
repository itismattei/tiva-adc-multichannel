/*
 * main.c
 */
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_adc.h"
#include "inc/hw_gpio.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "dist.h"
#include "driverlib/timer.h"

#include "driverlib/uart.h"
#include "uartp/uart.h"
#include "uartp/uartstdio.h"
#include "uartp/cmdline.h"

void console_init(void);


#define ADC_SEQ                 (ADC_O_SSMUX0)
#define ADC_SEQ_STEP            (ADC_O_SSMUX1 - ADC_O_SSMUX0)
#define ADC_SSMUX               (ADC_O_SSMUX0 - ADC_O_SSMUX0)
#define ADC_SSEMUX              (ADC_O_SSEMUX0 - ADC_O_SSMUX0)
#define ADC_SSCTL               (ADC_O_SSCTL0 - ADC_O_SSMUX0)
#define ADC_SSFIFO              (ADC_O_SSFIFO0 - ADC_O_SSMUX0)
#define ADC_SSFSTAT             (ADC_O_SSFSTAT0 - ADC_O_SSMUX0)
#define ADC_SSOP                (ADC_O_SSOP0 - ADC_O_SSMUX0)
#define ADC_SSDC                (ADC_O_SSDC0 - ADC_O_SSMUX0)



int32_t ADCSequenceData_Get(uint32_t ui32Base, uint32_t ui32SequenceNum, uint32_t *pui32Buffer){

    uint32_t ui32Count;

    //
    // Check the arguments.
    //
    ASSERT((ui32Base == ADC0_BASE) || (ui32Base == ADC1_BASE));
    ASSERT(ui32SequenceNum < 4);

    //
    // Get the offset of the sequence to be read.
    //
    ui32Base += ADC_SEQ + (ADC_SEQ_STEP * ui32SequenceNum);

    //
    // Read samples from the FIFO until it is empty.
    //
    ui32Count = 0;
    while(!(HWREG(ui32Base + ADC_SSFSTAT) & ADC_SSFSTAT0_EMPTY) &&
          (ui32Count < 10))
    {
        //
        // Read the FIFO and copy it to the destination.
        //
        *pui32Buffer++ = HWREG(ui32Base + ADC_SSFIFO);

        //
        // Increment the count of samples read.
        //
        ui32Count++;
    }

    //
    // Return the number of samples read.
    //
    return(ui32Count);
}


distanza D;
volatile uint32_t numByte;

void adcISR(void){
	volatile uint32_t attesa;
	ADCIntClear(ADC0_BASE, 0);
	numByte = ADCSequenceData_Get(ADC0_BASE, 0, D.dI);    // Read ADC Value.
	/// riavvia il campionamento
	//HWREG(ADC0_BASE + ADC_O_PSSI) |= ((2 & 0xffff0000) | (1 << (2 & 0xf)));
	HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + (GPIO_PIN_0 << 2))) &=  ~GPIO_PIN_0;
	//ADCProcessorTrigger(ADC0_BASE, 0);
	for(attesa = 0; attesa < 1000; attesa++);
	HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + (GPIO_PIN_0 << 2))) |=  GPIO_PIN_0;
	for(attesa= 0; attesa<6; attesa++)
	{
		PRINTF("%d \t", D.dI[attesa]);
	}
	PRINTF("\n");
}


void Timer0ISR(void){

    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ADCProcessorTrigger(ADC0_BASE, 0);
}

void main(){


	console_init();

	initTimer0();

	uint32_t adc1buffer[8];
	volatile uint32_t ch0data, ch1data;
	//int i,Average0,ch0data0,ch0data1,ch0data2,ch0data3,Average1,ch1data0,ch1data1,ch1data2,ch1data3;
	SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);//20Mhz clock

	//setup PB0 per scopi di debug
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	/// PB0 in uscita
	HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) 	|= GPIO_PIN_0;
	/// no alternate function
	HWREG(GPIO_PORTB_BASE + GPIO_O_AFSEL) 	&= ~GPIO_PIN_0;
	/// 2 ma di corrente
	HWREG(GPIO_PORTB_BASE + GPIO_O_DR2R)	|= GPIO_PIN_0;
	/// controllo slew rate off
	HWREG(GPIO_PORTB_BASE + GPIO_O_SLR)		&= ~GPIO_PIN_0;
	/// pull up
	//HWREG(GPIO_PORTF_BASE + GPIO_O_PUR)		|= GPIO_PIN_4;
	/// abilitazione della funzione digitale
	HWREG(GPIO_PORTB_BASE + GPIO_O_DEN)		|= GPIO_PIN_0;
	/// imposta il pin (ricordare lo shift di 2 posizione verso sinistra della maschera di bit
	HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + (GPIO_PIN_0 << 2))) |=  GPIO_PIN_0;
	HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + (GPIO_PIN_0 << 2))) &=  ~GPIO_PIN_0;




	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
//SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	ADCReferenceSet(ADC0_BASE, ADC_REF_INT); //Set reference to the internal reference ,You can set it to 1V or 3 V
//ADCReferenceSet(ADC1_BASE, ADC_REF_INT);
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3); //Ain0
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2); //Ain1
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1); //Ain2
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0); //Ain3
	GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_3); //Ain4
	/// imposta il sequencer 0, che ha 8 letture
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);

	ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0);
	/// PE.2
	ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH1);

	// PE.1
	ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH3 );
	//GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0); //Ain3
	// PE.0
	ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_CH3 );
	/// PD.3
	ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ADC_CTL_CH4 | ADC_CTL_IE | ADC_CTL_END);

	/// abilita il sequencer 0
	ADCSequenceEnable(ADC0_BASE, 0);

	//ABILITAZIONE INTERRUZIONI//

	/// abilta l'interruzione del sequencer 0
	ADCIntClear(ADC0_BASE, 0);
    //
    // Enable the ADC interrupt.
    //
    ROM_IntEnable(INT_ADC0SS0);
    ADCIntEnable(ADC0_BASE, 0);
    //
    // Enable processor interrupts.
    //
    ROM_IntMasterEnable();

	while(1){

	}
}





void console_init(void)
{
    //
    // Enable the peripherals used by this example.
    //
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //
    // Set GPIO A0 and A1 as UART pins.
    //
	ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
	ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
	ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
	ROM_GPIOPinConfigure(GPIO_PB1_U1TX);
	ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	//
    // Configure the UART for 115,200, 8-N-1 operation.
    //
	ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

	ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
}
