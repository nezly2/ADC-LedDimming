#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"
#include "fsl_tpm.h"
#include "fsl_adc16.h"
#include "TPMDuty.h"

int main(void){

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    BOARD_InitDebugConsole();
#endif

    adc16_config_t adc16ConfigStruct;//configurar el ADC
	adc16_channel_config_t adc16ChannelConfigStruct[3];//Configurar
	ADC16_GetDefaultConfig(&adc16ConfigStruct);//
	adc16ConfigStruct.resolution = kADC16_ResolutionSE8Bit;//
	ADC16_Init(ADC0, &adc16ConfigStruct);//Inicia el ADC0
	ADC16_EnableHardwareTrigger(ADC0, false);//
	ADC16_DoAutoCalibration(ADC0);//Función para calibrar el ADC
	adc16ChannelConfigStruct[0].channelNumber = 0;//Para escoger el canal E20
	adc16ChannelConfigStruct[0].enableInterruptOnConversionCompleted = false;
	adc16ChannelConfigStruct[0].enableDifferentialConversion = false;
	adc16ChannelConfigStruct[1].channelNumber = 4;//Para escoger el canal E21
	adc16ChannelConfigStruct[1].enableInterruptOnConversionCompleted = false;
	adc16ChannelConfigStruct[1].enableDifferentialConversion = false;
	adc16ChannelConfigStruct[2].channelNumber = 3;//Para escoger el canal E22
	adc16ChannelConfigStruct[2].enableInterruptOnConversionCompleted = false;
	adc16ChannelConfigStruct[2].enableDifferentialConversion = false;

    //Checar capitulo 10.3 pagina 161 del users manual
    tpm_config_t tpmInfo; //variable para configurar el timer
    tpm_chnl_pwm_signal_param_t tpmParam[3]; //variable para configurar los 3 canales de los PWM de B18, B19 y D1

    /* Configure tpm params with frequency 24kHZ */
    tpmParam[0].chnlNumber = (tpm_chnl_t)0u; // canal 1 del timer como salida para B18
    tpmParam[1].chnlNumber = (tpm_chnl_t)1u; // canal 1 del timer como salida para B19
    tpmParam[2].chnlNumber = (tpm_chnl_t)1u; // canal 1 del timer como salida para D1
    tpmParam[0].level = kTPM_LowTrue;//comienza en cero el pwm
    tpmParam[1].level = kTPM_LowTrue;//comienza en cero el pwm
    tpmParam[2].level = kTPM_LowTrue;//comienza en cero el pwm
    tpmParam[0].dutyCyclePercent = 0;
    tpmParam[1].dutyCyclePercent = 0;
    tpmParam[2].dutyCyclePercent = 0;
    CLOCK_SetTpmClock(1U);/* Select  kCLOCK_PllFllSelClk */
	TPM_GetDefaultConfig(&tpmInfo);
	TPM_Init(TPM2, &tpmInfo); //TPM pines B18, B19
	TPM_Init(TPM0, &tpmInfo); //TPM pin D1

	TPM_SetupPwm(TPM2, &tpmParam[0], 1U, kTPM_EdgeAlignedPwm, 24000U, CLOCK_GetFreq(kCLOCK_PllFllSelClk));
	TPM_SetupPwm(TPM2, &tpmParam[1], 1U, kTPM_EdgeAlignedPwm, 24000U, CLOCK_GetFreq(kCLOCK_PllFllSelClk));
	TPM_SetupPwm(TPM0, &tpmParam[2], 1U, kTPM_EdgeAlignedPwm, 24000U, CLOCK_GetFreq(kCLOCK_PllFllSelClk));
	TPM_StartTimer(TPM2, kTPM_SystemClock);
	TPM_StartTimer(TPM0, kTPM_SystemClock);
    int i1, i2, i3;

    while(1) {
    	ADC16_SetChannelConfig(ADC0, 0, &adc16ChannelConfigStruct[0]);
		while (0U == (kADC16_ChannelConversionDoneFlag & ADC16_GetChannelStatusFlags(ADC0, 0))){}
		i1 = ADC16_GetChannelConversionValue(ADC0, 0);
		ADC16_SetChannelConfig(ADC0, 0, &adc16ChannelConfigStruct[1]);
		while (0U == (kADC16_ChannelConversionDoneFlag & ADC16_GetChannelStatusFlags(ADC0, 0))){}
		i2 = ADC16_GetChannelConversionValue(ADC0, 0);
		ADC16_SetChannelConfig(ADC0, 0, &adc16ChannelConfigStruct[2]);
		while (0U == (kADC16_ChannelConversionDoneFlag & ADC16_GetChannelStatusFlags(ADC0, 0))){}
		i3 = ADC16_GetChannelConversionValue(ADC0, 0);

        /* Update PWM duty cycle */
        TPM_Update(TPM2, (tpm_chnl_t)0u, kTPM_EdgeAlignedPwm,i1);
        TPM_Update(TPM2, (tpm_chnl_t)1u, kTPM_EdgeAlignedPwm,i2);
        TPM_Update(TPM0, (tpm_chnl_t)1u, kTPM_EdgeAlignedPwm,i3);
    }
}
