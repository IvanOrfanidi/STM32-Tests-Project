
#include "gpio.h"
#include "stm32l0xx_hal.h"

/** Configure pins as
 * Analog
 * Input
 * Output
 * EVENT_OUT
 * EXTI
 */
void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    //__HAL_RCC_GPIOH_CLK_ENABLE();
    //__HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : PC13 */
    /*GPIO_InitStruct.Pin = GPIO_PIN_13;
   GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);*/

    /*Configure GPIO pin : LD2_Pin */
    GPIO_InitStruct.Pin = LD2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);
}

void EXTI5_15_IRQHandler_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIOC clock */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /* Configure PC.13 pin as input floating */
    GPIO_InitStructure.Mode = GPIO_MODE_EVT_FALLING;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Enable and set EXTI lines 4 to 15 Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_13) {
        extern LPTIM_HandleTypeDef hlptim1;
        printf("Exti\r\nThe Pulse number = %ld\r\n", HAL_LPTIM_ReadCounter(&hlptim1));
    }
}