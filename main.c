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


#define ADC_SEQ                 (ADC_O_SSMUX0)
#define ADC_SEQ_STEP            (ADC_O_SSMUX1 - ADC_O_SSMUX0)
#define ADC_SSMUX               (ADC_O_SSMUX0 - ADC_O_SSMUX0)
#define ADC_SSEMUX              (ADC_O_SSEMUX0 - ADC_O_SSMUX0)
#define ADC_SSCTL               (ADC_O_SSCTL0 - ADC_O_SSMUX0)
#define ADC_SSFIFO              (ADC_O_SSFIFO0 - ADC_O_SSMUX0)
#define ADC_SSFSTAT             (ADC_O_SSFSTAT0 - ADC_O_SSMUX0)
#define ADC_SSOP                (ADC_O_SSOP0 - ADC_O_SSMUX0)
#define ADC_SSDC                (ADC_O_SSDC0 - ADC_O_SSMUX0)

/*main(void) {

	uint32_t pui32ADC0Value[1];

	SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN| SYSCTL_XTAL_16MHZ);                                                                            //Set clock at 40 Mhz , Sometimes                                                                          //ADC may not work at 80Mhz
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	ADCReferenceSet(ADC0_BASE, ADC_REF_INT); //Set reference to the internal reference
											// You can set it to 1V or 3 V
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5); //Configure GPIO as ADC

	ADCSequenceDisable(ADC0_BASE, 3); //It is always a good practice to disable ADC prior                                                        //to usage ,else the ADC may not be accurate                                                               //   due to previous initializations
	ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0); //Use the 3rd Sample sequencer

	ADCSequenceStepConfigure(ADC0_BASE, 3, 0,ADC_CTL_CH8 | ADC_CTL_IE | ADC_CTL_END);
															 //Configure ADC to read from channel 8 ,trigger the interrupt to end data capture //

	ADCSequenceEnable(ADC0_BASE, 3);   //Enable the ADC
	ADCIntClear(ADC0_BASE, 3);     //Clear interrupt to proceed to  data capture

	while (1) {
		ADCProcessorTrigger(ADC0_BASE, 3);   //Ask processor to trigger ADC
		while (!ADCIntStatus(ADC0_BASE, 3, false)){ //Do nothing until interrupt is triggered
		}
	
		ADCIntClear(ADC0_BASE, 3); //Clear Interrupt to proceed to next data capture
		ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value); //pui32ADC0Value is the value read

		SysCtlDelay(SysCtlClockGet() / 12);
	} //Suitable delay
}
*/


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


uint32_t adc0buffer[12];
volatile uint32_t numByte;

void adcISR(void){

	ADCIntClear(ADC0_BASE, 0);
	numByte = ADCSequenceData_Get(ADC0_BASE, 0, adc0buffer);    // Read ADC Value.
	/// riavvia il campionamento
	//HWREG(ADC0_BASE + ADC_O_PSSI) |= ((2 & 0xffff0000) | (1 << (2 & 0xf)));
	//HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + (GPIO_PIN_0 << 2))) &=  ~GPIO_PIN_0;
	ADCProcessorTrigger(ADC0_BASE, 0);
	//HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + (GPIO_PIN_0 << 2))) |=  GPIO_PIN_0;
}

void main(){
	uint32_t adc1buffer[8];
	volatile uint32_t ch0data, ch1data;
	//int i,Average0,ch0data0,ch0data1,ch0data2,ch0data3,Average1,ch1data0,ch1data1,ch1data2,ch1data3;
	SysCtlClockSet(SYSCTL_SYSDIV_10|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);//20Mhz clock

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
	ADCReferenceSet(ADC0_BASE, ADC_REF_INT); //Set reference to the internal reference ,You can set it to 1V or 3 V
//ADCReferenceSet(ADC1_BASE, ADC_REF_INT);
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3); //Ain0
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2); //Ain1
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1); //Ain2
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0); //Ain3
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5); //Ain8
	/// imposta il sequencer 0, che ha 8 letture
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	//
	// Set the trigger event for this sample sequence.
	//
	/*HWREG(ADC0_BASE + ADC_O_EMUX) = ((HWREG(ADC0_BASE + ADC_O_EMUX) &
									 ~(0xf << (2*4))) |
									((0 & 0xf) << (2*4)));

	/// imposta la priorità
	HWREG(ADC0_BASE + ADC_O_SSPRI) = ((HWREG(ADC0_BASE + ADC_O_SSPRI) &
	                                      ~(0xf << (2*4))) |
	                                     ((0 & 0x3) << (2*4)));*/
//ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
//ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	/// PE.2
	//ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH1);
	/// legge nell'ordine il canale 2,il 3 e il n.8

	/// PE.0
	//ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH3);
	/// scrive il numero del canale all'interno dei 4 bit del mux0 del registro ADCSSMUX2
	/*HWREG(ADC0_BASE + 0x40 + (0x60 - 0x40)*2 + ADC_SSMUX) = ((HWREG(ADC0_BASE + 0x40 + (0x60 - 0x40)*2 + ADC_SSMUX) &
	                                    ~(0x0000000f << 0)) | ((ADC_CTL_CH3 & 0x0f) << 0));

	HWREG(ADC0_BASE + 0x40 + (0x60 - 0x40)*2 + ADC_SSEMUX) = ((HWREG(ADC0_BASE + 0x40 + (0x60 - 0x40)*2 + ADC_SSEMUX) &
	                                     ~(0x0000000f << 0)) | (((ADC_CTL_CH3 & 0xf00) >> 8) << 0));

	HWREG(ADC0_BASE + 0x40 + (0x60 - 0x40)*2  + ADC_SSCTL) = ((HWREG(ADC0_BASE + 0x40 + (0x60 - 0x40)*2  + ADC_SSCTL) &
	                                     ~(0x0000000f << 0)) | (((ADC_CTL_CH3 & 0xf0) >> 4) << 0));

	HWREG(ADC0_BASE + 0x40 + (0x60 - 0x40)*2 + ADC_SSOP) &= ~(1 << 0);*/
	/// PE.3
	ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH1);
	/// PE.2
	ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH0);

	// PE.1
	ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH2 );
	//GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0); //Ain3
	// PE.0
	ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_CH8 );
	/// PE.5
	ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);
/*ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0);
ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH0);
ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH0);
ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_CH0);
ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ADC_CTL_CH0);
ADCSequenceStepConfigure(ADC0_BASE, 0, 5, ADC_CTL_CH0);
ADCSequenceStepConfigure(ADC0_BASE, 0, 6, ADC_CTL_CH0);
ADCSequenceStepConfigure(ADC0_BASE, 0, 7, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
ADCSequenceStepConfigure(ADC1_BASE, 0, 0, ADC_CTL_CH1);
ADCSequenceStepConfigure(ADC1_BASE, 0, 1, ADC_CTL_CH1);
ADCSequenceStepConfigure(ADC1_BASE, 0, 2, ADC_CTL_CH1);
ADCSequenceStepConfigure(ADC1_BASE, 0, 3, ADC_CTL_CH1);
ADCSequenceStepConfigure(ADC1_BASE, 0, 4, ADC_CTL_CH1);
ADCSequenceStepConfigure(ADC1_BASE, 0, 5, ADC_CTL_CH1);
ADCSequenceStepConfigure(ADC1_BASE, 0, 6, ADC_CTL_CH1);
ADCSequenceStepConfigure(ADC1_BASE, 0, 7, ADC_CTL_CH1 | ADC_CTL_IE | ADC_CTL_END);*/
	/// abilita il sequencer 1
	ADCSequenceEnable(ADC0_BASE, 0);
	//
	// Enable the specified sequence.
	//
	//HWREG(ADC0_BASE + ADC_O_ACTSS) |= 1 << 2;
//ADCSequenceEnable(ADC0_BASE, 0);
//ADCSequenceEnable(ADC1_BASE, 0);
	/// abilta l'interruzione del sequencer2
	ADCIntClear(ADC0_BASE, 0);
//ADCIntClear(ADC0_BASE, 0);
//ADCIntClear(ADC1_BASE, 0);
/*Kentec320x240x16_SSD2119Init();
GrContextInit(&sContext, &g_sKentec320x240x16_SSD2119);
ClrScreen();
GrContextForegroundSet(&sContext, ClrBlue);
GrContextFontSet(&sContext, &g_sFontCmsc20);
GrStringDraw(&sContext, "ADC Value:" , -1, 5, 22, 0);
GrStringDraw(&sContext, "ADC Value:" , -1, 5, 52, 0);
GrFlush(&sContext);*/

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
		ADCIntClear(ADC0_BASE, 0);
		// Trigger the ADC conversion on sequencer 0
		ADCProcessorTrigger(ADC0_BASE, 0);
		//HWREG(ADC0_BASE + ADC_O_PSSI) |= ((2 & 0xffff0000) | (1 << (2 & 0xf)));
		/// polling
		while(!ADCIntStatus(ADC0_BASE, 0, false)){}        // Wait for conversion to be completed.
		ADCIntClear(ADC0_BASE, 0);                        // Clear the ADC interrupt flag.
		for(;;);
		//ch2data = adcbuffer[0];
		//ch3data = adcbuffer[1];
		//ch8data = adcbuffer[2];
		/*for (i=0;i<16;i++){
			ADCIntClear(ADC0_BASE, 0);
			ADCIntClear(ADC1_BASE, 0);
			ADCProcessorTrigger(ADC1_BASE, 0|ADC_TRIGGER_WAIT);
			ADCProcessorTrigger(ADC0_BASE, 0|ADC_TRIGGER_SIGNAL);
			while(!ADCIntStatus(ADC0_BASE, 0, false)){}            // Wait for conversion to be completed.
			ADCIntClear(ADC0_BASE, 0);                    // Clear the ADC interrupt flag.
			ADCIntClear(ADC1_BASE, 0);                    // Clear the ADC interrupt flag.
			ADCSequenceDataGet(ADC0_BASE, 0, adc0buffer);            // Read ADC Value.
			ADCSequenceDataGet(ADC0_BASE, 0, adc1buffer);            // Read ADC Value.
			ch0data = (adc0buffer[0] + adc0buffer[1] + adc0buffer[2] + adc0buffer[3] + adc0buffer[4] + adc0buffer[5] + adc0buffer[6] + adc0buffer[7])/8;
			ch1data = (adc1buffer[0] + adc1buffer[1] + adc1buffer[2] + adc1buffer[3] + adc1buffer[4] + adc1buffer[5] + adc1buffer[6] + adc1buffer[7])/8;
			Average0 = ch0data;
			Average1 = ch1data;
			ch0data0 = Average0/1000;
			ch0data1 = Average0%1000/100;
			ch0data2 = Average0%1000%100/10;
			ch0data3 = Average0%1000%100%10/1;
			ch1data0 = Average1/1000;
			ch1data1 = Average1%1000/100;
			ch1data2 = Average1%1000%100/10;
			ch1data3 = Average1%1000%100%10/1;
			char buffer0[10],buffer1[10],buffer2[10],buffer3[10];
			char buffer4[10],buffer5[10],buffer6[10],buffer7[10];
			sprintf(buffer0,"%d",ch0data0);
			sprintf(buffer1,"%d",ch0data1);
			sprintf(buffer2,"%d",ch0data2);
			sprintf(buffer3,"%d",ch0data3);
			sprintf(buffer4,"%d",ch1data0);
			sprintf(buffer5,"%d",ch1data1);
			sprintf(buffer6,"%d",ch1data2);
			sprintf(buffer7,"%d",ch1data3);
			SysCtlDelay(SysCtlClockGet()/3);
			ClrScreen2();
			ClrScreen3();
			GrContextForegroundSet(&sContext, ClrRed);
			GrContextFontSet(&sContext, &g_sFontCmsc20);
			GrStringDraw(&sContext, buffer0 , -1, 130, 22, 0);
			GrStringDraw(&sContext, buffer1 , -1, 140, 22, 0);
			GrStringDraw(&sContext, buffer2 , -1, 150, 22, 0);
			GrStringDraw(&sContext, buffer3 , -1, 160, 22, 0);
			GrStringDraw(&sContext, buffer4 , -1, 130, 52, 0);
			GrStringDraw(&sContext, buffer5 , -1, 140, 52, 0);
			GrStringDraw(&sContext, buffer6 , -1, 150, 52, 0);
			GrStringDraw(&sContext, buffer7 , -1, 160, 52, 0);
			GrFlush(&sContext);
		}*/
	}
}
