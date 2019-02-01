
#include "includes.h"

ADCmeasCFG stAdcSource;

static void adcCalibration();

void GetAdcValue(ADCmeasCFG* pstAdcSource)
{
    NVIC_DisableIRQ(ADC1_2_IRQn);
    pstAdcSource->usBatMeasValue = stAdcSource.usBatMeasValue;
    pstAdcSource->usWkupPinValue = stAdcSource.usWkupPinValue;
    NVIC_EnableIRQ(ADC1_2_IRQn);
}

void ADC_Configuration(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* Configure as analog input Bat Meas -------------------------*/
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configures NVIC and Vector Table base location -------------------------*/
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_EnableIRQ(ADC1_2_IRQn);

    /* ADC1 Configuration */
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE);

    /* ADC1 Configuration ------------------------------------------------------*/
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    ADC_Init(ADC2, &ADC_InitStructure);

    /* ADC1 regular channels configuration */
    ADC_InjectedSequencerLengthConfig(ADC1, 1);
    ADC_InjectedSequencerLengthConfig(ADC2, 1);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC2, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);

    ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_None);
    ADC_ExternalTrigInjectedConvConfig(ADC2, ADC_ExternalTrigInjecConv_None);
    // Enable automatic injected conversion start after regular one
    ADC_AutoInjectedConvCmd(ADC1, ENABLE);
    ADC_AutoInjectedConvCmd(ADC2, ENABLE);
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
    ADC_ITConfig(ADC2, ADC_IT_EOC, ENABLE);

    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);
    ADC_Cmd(ADC2, ENABLE);

    adcCalibration();

    /* Start ADC1 Software Conversion */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    ADC_SoftwareStartConvCmd(ADC2, ENABLE);
}

static void adcCalibration(void)
{
#ifdef ADC_CALIBRATION
    /* Enable ADC1 reset calibration register */
    ADC_ResetCalibration(ADC1);
    ADC_ResetCalibration(ADC2);
    /* Check the end of ADC1 reset calibration register */
    while(ADC_GetResetCalibrationStatus(ADC1))
        ;
    while(ADC_GetResetCalibrationStatus(ADC2))
        ;

    /* Start ADC1 calibration */
    ADC_StartCalibration(ADC1);
    ADC_StartCalibration(ADC2);
    /* Check the end of ADC1 calibration */
    while(ADC_GetCalibrationStatus(ADC1))
        ;
    while(ADC_GetCalibrationStatus(ADC2))
        ;
#endif
}

void ADC1_2_IRQHandler(void)
{
    if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) {
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
        stAdcSource.usBatMeasValue = ADC_GetConversionValue(ADC1);
    }

    if(ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC)) {
        ADC_ClearITPendingBit(ADC2, ADC_IT_EOC);
        stAdcSource.usWkupPinValue = ADC_GetConversionValue(ADC2);
    }

    if(ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC))
        ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
    if(ADC_GetFlagStatus(ADC2, ADC_FLAG_JEOC))
        ADC_ClearITPendingBit(ADC2, ADC_IT_JEOC);
}
