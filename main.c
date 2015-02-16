/*
 * main.c
 */
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"

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

void main(){
	uint32_t adc0buffer[8],adc1buffer[8];
	volatile uint32_t ch0data, ch1data;
	//int i,Average0,ch0data0,ch0data1,ch0data2,ch0data3,Average1,ch1data0,ch1data1,ch1data2,ch1data3;
	SysCtlClockSet(SYSCTL_SYSDIV_10|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);//20Mhz clock
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
//SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	ADCReferenceSet(ADC0_BASE, ADC_REF_INT); //Set reference to the internal reference ,You can set it to 1V or 3 V
//ADCReferenceSet(ADC1_BASE, ADC_REF_INT);
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3); //Configure GPIO as ADC
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);
	/// imposta il sequencer 1, che ha 4 letture
	ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);
//ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
//ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	/// legge nell'ordine il canale 2,il 3 e il n.8
	/// PE.1
	ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH2);
	// PE.0
	ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_CH3);
	/// PE.5
	ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_CH8 | ADC_CTL_IE | ADC_CTL_END);
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
	/// abilita il sequencer 2
	ADCSequenceEnable(ADC0_BASE, 2);
//ADCSequenceEnable(ADC0_BASE, 0);
//ADCSequenceEnable(ADC1_BASE, 0);
	/// abilta l'interruzione del sequencer2
	ADCIntClear(ADC0_BASE, 2);
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
	while(1){
		ADCIntClear(ADC0_BASE, 2);
		// Trigger the ADC conversion on sequencer 2
		ADCProcessorTrigger(ADC0_BASE, 2);
		/// polling
		while(!ADCIntStatus(ADC0_BASE, 2, false)){}        // Wait for conversion to be completed.
		ADCIntClear(ADC0_BASE, 2);                        // Clear the ADC interrupt flag.
		ADCSequenceDataGet(ADC0_BASE, 2, adc0buffer);    // Read ADC Value.
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
