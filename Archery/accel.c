

#include "includes.h"
#include "accel.h"

#define ACCEL_THRESACT 3000 / 62.5     //Установка порога Толчка
#define ACCEL_THRESHOLD 3000 / 62.5    //Установка порога Толчка
#define ACCEL_DURATION 20000 / 625     //Установка лимита времени для Толчка
#define ACCEL_LATENT 10 / 1.25         //Установка задержки для Толчка
#define ACCEL_WINDOWS 60 / 1.25        //Установка временного окна для Второго Толчка
#define ACCEL_TIME_INACTIVITY 30       //Установка времени Неактивности

void vAccelTask(void* pvParameters)
{
    portTickType xLastWakeTimerDelay;
    static uint16_t usTimeoutCountStep = 0;
    static uint16_t usTimeoutStepReset = 0;
    static _Bool bTap = FALSE;

    static _Bool AxisX = 0, AxisY = 0, AxisZ = 0;
    int16_t a_sAccelValue[3];
    // char strMsgDebug[128];
    static uint8_t eStepShotHandler = 0;
    AccelInit();
    Accel_Clear_Settings();

#ifdef ACCEL_THRESACT
    Set_Activity_Threshold(ACCEL_THRESACT);
#endif
    Set_Tap_Threshold(ACCEL_THRESHOLD);    //Установка порога Толчка
    Set_Tap_Duration(ACCEL_DURATION);      //Установка лимита времени для Толчка
    Set_Tap_Latency(ACCEL_LATENT);         //Установка задержки для Толчка
    Set_Tap_Window(ACCEL_WINDOWS);         //Установка временного окна для Второго Толчка
#ifdef ACCEL_TIME_INACTIVITY
    Set_Time_Inactivity(ACCEL_TIME_INACTIVITY);    //Установка времени Неактивности
#endif
    Reset_Interrupt();

    Set_Interrupt(SINGLE_TAP, 1, INT_1);
    Set_Interrupt(DOUBLE_TAP, 1, INT_2);

    while(1) {
        if(!(ADXL345_ReadXYZ(a_sAccelValue))) {
            g_stAccelData.sValueAxisX = a_sAccelValue[AXIS_X] * -1;
            g_stAccelData.sValueAxisY = a_sAccelValue[AXIS_Y] * -1;
            g_stAccelData.sValueAxisZ = a_sAccelValue[AXIS_Z] * -1;
            g_stAccelData.ucInterrupt = Get_Source_Interrupt();

            if(g_stAccelData.ucInterrupt & (SINGLE_TAP | DOUBLE_TAP)) {
                if(eStepShotHandler) {
                    bTap = TRUE;
                }
            }
            g_stAccelData.bDataValid = 1;
        }
        else {
            g_stAccelData.bDataValid = 0;
        }
        // sprintf(strMsgDebug, "%+d\r\n", g_stAccelData.sValueAxisX);
        if(g_eStatusCharging == CHARGING_OFF) {
            if(eStepShotHandler == 0) {
                if(g_stAccelData.sValueAxisZ > 110 && g_stAccelData.sValueAxisZ < 210) {
                    AxisZ = 1;
                }
                else {
                    AxisZ = 0;
                }
                if(g_stAccelData.sValueAxisX < 380 && g_stAccelData.sValueAxisX > 200) {
                    AxisX = 1;
                }
                else {
                    AxisX = 0;
                }
                if(g_stAccelData.sValueAxisY > 200 && g_stAccelData.sValueAxisY < 400) {
                    AxisY = 1;
                }
                else {
                    AxisY = 0;
                }

                /* Ждем пока все устаканится, лук поднят и находится в горизонтальной плоскости */
                if(AxisX && AxisY && AxisZ) {
                    usTimeoutCountStep++;
                    if(usTimeoutCountStep > 1000) {    // 800
                        usTimeoutCountStep = 0;
                        usTimeoutStepReset = 0;
                        eStepShotHandler = 1;
                        USART_Write(UART_DBG, "Step #1\r\n", strlen("Step #1\r\n"));
                    }
                }
                else {
                    usTimeoutCountStep = 0;
                }
            }

            /* Ждем опракидывания лука в плоскости стрельбы после выстрела. */
            if(eStepShotHandler == 1) {
                if(g_stAccelData.sValueAxisZ < 120) {
                    AxisZ = 1;
                }
                else {
                    AxisZ = 0;
                }
                if(g_stAccelData.sValueAxisX < 200) {
                    AxisX = 1;
                }
                else {
                    AxisX = 0;
                }
                if(g_stAccelData.sValueAxisY > 200 && g_stAccelData.sValueAxisY < 500) {
                    AxisY = 1;
                }
                else {
                    AxisY = 0;
                }

                if(AxisX && AxisY && AxisZ) {
                    usTimeoutCountStep++;
                    if(usTimeoutCountStep > 100) {    // 50
                        usTimeoutCountStep = 0;
                        eStepShotHandler = 0;
                        usTimeoutStepReset = 0;
                        if(bTap) {
                            IncreaseCountShot();
                            USART_Write(UART_DBG, "Tap TRUE\r\n", strlen("Tap TRUE\r\n"));
                        }
                        bTap = FALSE;
                        USART_Write(UART_DBG, "Step #2\r\n", strlen("Step #2\r\n"));
                    }
                }
                else {
                    usTimeoutCountStep = 0;
                }
            }

            /* Лук стоит на стойке в вертикальном положении, сброс шага */
            if(g_stAccelData.sValueAxisZ < 0 || g_stAccelData.sValueAxisX > 500) {
                usTimeoutStepReset++;
                if(usTimeoutStepReset > 3000) {
                    eStepShotHandler = 0;
                    usTimeoutStepReset = 0;
                    bTap = FALSE;
                    USART_Write(UART_DBG, "Rest Step\r\n", strlen("Rest Step\r\n"));
                }
            }
        }
        else {
            usTimeoutStepReset = 0;
            usTimeoutCountStep = 0;
            eStepShotHandler = 0;
            bTap = FALSE;
        }

        xLastWakeTimerDelay = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTimerDelay, (1 / portTICK_RATE_MS));
    }
}
