
#include "includes.h"

#define RESET_STEP \
    eStepShotHandler = 0; \
    usTimeoutStepReset = 0; \
    usTimeoutCountStep = 0; \
    bTap = FALSE; \
    usTimeoutTapReset = 0

static void increaseCountShot();

int g_usShot = 0;                 // счетчик выстрелов
_Bool g_bAccelActiv = DISABLE;    // флаг включения акселерометра его для обработки

/*
#define NUM_SEC_ACCEL_ARV       4       //sec
#define SIZE_BUF_ACCEL_ARV      (((NUM_SEC_ACCEL_ARV/2)* configTICK_RATE_HZ) * NUM_AXIS)
volatile uint16_t AccelDataArchiveBeforeShot[SIZE_BUF_ACCEL_ARV];
volatile uint16_t AccelDataArchiveAfterShot[SIZE_BUF_ACCEL_ARV];
uint16_t IndexArchiveBeforeShot;
*/

void vAccelTask(void* pvParameters)
{
    portTickType xLastWakeTimerDelay;
    static uint16_t usTimeoutCountStep = 0;
    static uint16_t usTimeoutStepReset = 0;
    static uint16_t usTimeoutTapReset = 0;
    static uint32_t uiTimeoutStep2 = 0;
    static uint8_t eStepShotHandler = 0;

    static int16_t sValueAxisX;
    static int16_t sValueAxisY;
    static int16_t sValueAxisZ;
    uint8_t ucInterrupt;

    static _Bool AxisX = 0;
    static _Bool AxisY = 0;
    static _Bool AxisZ = 0;
    static _Bool bTap = FALSE;

    g_usShot = getShot();    // Вычитываем из памяти сколько было выстрелов до запуска.

    AccelInit();
    Accel_Clear_Settings();
#ifdef ACCEL_THRESACT
    Set_Activity_Threshold(ACCEL_THRESACT);
#endif
#ifdef ACCEL_THRESHOLD
    Set_Tap_Threshold(ACCEL_THRESHOLD);    //Установка порога Толчка
#endif
#ifdef ACCEL_DURATION
    Set_Tap_Duration(ACCEL_DURATION);    //Установка лимита времени для Толчка
#endif
#ifdef ACCEL_LATENT
    Set_Tap_Latency(ACCEL_LATENT);    //Установка задержки для Толчка
#endif
#ifdef ACCEL_WINDOWS
    Set_Tap_Window(ACCEL_WINDOWS);    //Установка временного окна для Второго Толчка
#endif
#ifdef ACCEL_TIME_INACTIVITY
    Set_Time_Inactivity(ACCEL_TIME_INACTIVITY);    //Установка времени Неактивности
#endif
    Reset_Interrupt();

#ifdef USE_ADXL345_INT1
    Set_Interrupt(SINGLE_TAP, 1, INT_2);
#endif

    while(1) {
        /* Читаем оси акселерометра */
        if(!(ADXL345_ReadXYZ(&sValueAxisX, &sValueAxisY, &sValueAxisZ))) {
            ucInterrupt = Get_Source_Interrupt();
        }
        else {
            RESET_STEP;
        }

        if(AccelState() == ENABLE) {
            if(!(eStepShotHandler)) {
                eStepShotHandler = 1;
            }
        }
        else {
            RESET_STEP;
        }

        if(eStepShotHandler == 1) {
            if(sValueAxisX > X_MIN_VALUE_STEP1 && sValueAxisX < X_MAX_VALUE_STEP1) {
                AxisX = 1;
            }
            else {
                AxisX = 0;
            }
            if(sValueAxisY > Y_MIN_VALUE_STEP1 && sValueAxisY < Y_MAX_VALUE_STEP1) {
                AxisY = 1;
            }
            else {
                AxisY = 0;
            }
            if(sValueAxisZ > Z_MIN_VALUE_STEP1 && sValueAxisZ < Z_MAX_VALUE_STEP1) {
                AxisZ = 1;
            }
            else {
                AxisZ = 0;
            }

            /* Ждем пока все устаканится, лук поднят и находится в горизонтальной плоскости */
            if(AxisX && AxisY && AxisZ) {
                usTimeoutCountStep++;
                if(usTimeoutCountStep > TIMEOUT_TRANSITION_STEP1) {
                    cout("Step #1\r\n");
                    usTimeoutCountStep = 0;
                    usTimeoutStepReset = 0;
                    eStepShotHandler = 2;
                    /*portTickType xTimeout = 100;
               xQueueSend(xBuzQueue, &xTimeout, 0);*/
                }
            }
            else {
                usTimeoutCountStep = 0;
            }
        }

        /* Ждем опракидывания лука в плоскости стрельбы после выстрела. */
        if(eStepShotHandler == 2) {
            if(sValueAxisX > X_VALUE_STEP2) {
                AxisX = 1;
            }
            else {
                AxisX = 0;
            }
            if(sValueAxisY > Y_MIN_VALUE_STEP2 && sValueAxisY < Y_MAX_VALUE_STEP2) {
                AxisY = 1;
            }
            else {
                AxisY = 0;
            }
            if(sValueAxisZ < Z_VALUE_STEP2) {
                AxisZ = 1;
            }
            else {
                AxisZ = 0;
            }

            /* сброс удара при зависании, считаем что это "помеха" */
            if(usTimeoutTapReset) {
                usTimeoutTapReset--;
                if(!(usTimeoutTapReset)) {
                    bTap = FALSE;
                    cout("Tap FALSE\r\n");
                }
            }

            /* проверка на удар */
            if(ucInterrupt & (SINGLE_TAP | DOUBLE_TAP)) {
                cout("Tap TRUE\r\n");
                bTap = TRUE;
                if(!(usTimeoutTapReset)) {
                    usTimeoutTapReset = MAX_TIMEOUT_TAP_RST;
                }
            }

            if(AxisX && AxisY && AxisZ) {
                usTimeoutCountStep++;
                if(usTimeoutCountStep > TIMEOUT_TRANSITION_STEP2) {    // 50
                    usTimeoutCountStep = 0;
                    eStepShotHandler = 0;
                    usTimeoutStepReset = 0;
                    cout("Step #2\r\n");
                    /*  проверка на удар */
                    if(bTap) {
                        bTap = FALSE;
                        portTickType xTimeout = 300;
                        xQueueSend(xBuzQueue, &xTimeout, 0);
                        increaseCountShot();
                    }
                    else {
                        RESET_STEP;
                    }
                }
            }
            else {
                usTimeoutCountStep = 0;
            }

            uiTimeoutStep2++;
            if(uiTimeoutStep2 > MAX_TIMEOUT_RST_STEP2) {
                // RESET_STEP;
            }
        }
        else {
            uiTimeoutStep2 = 0;
        }

        _Bool reset_step = FALSE;
        /* Лук сильно смещается в боковых направлениях, сброс шага */
        if(eStepShotHandler != 2 && (sValueAxisY > Y_MAX_VALUE_STEP_RST || sValueAxisY < Y_MIN_VALUE_STEP_RST)) {
            reset_step = TRUE;
        }

        /* Лук стоит на стойке в вертикальном положении, сброс шага */
        if(sValueAxisX < X_VALUE_STEP_RST) {
            reset_step = TRUE;
        }

        if(reset_step) {
            usTimeoutStepReset++;
            if(usTimeoutStepReset > MAX_TIMEOUT_STEP_RST) {
                RESET_STEP;
                usTimeoutStepReset = MAX_TIMEOUT_STEP_RST;
            }
        }
        else {
            usTimeoutStepReset = 0;
        }

        /*static int interval = 0;
      if(interval > 200) {
        interval = 0;

        cout("Step #%d\r\n", eStepShotHandler);
        cout("AxisX = %d  AxisY = %d  AxisZ = %d\r\n\r\n",
             AxisX,
             AxisY,
             AxisZ);
        
        if(!(AxisX)) cout("Value X = %d\r\n", sValueAxisX);
        if(!(AxisY)) cout("Value Y = %d\r\n", sValueAxisY);
        if(!(AxisZ)) cout("Value Z = %d\r\n", sValueAxisZ);

      }
      interval++;*/

        xLastWakeTimerDelay = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTimerDelay, (1 / portTICK_RATE_MS));
    }
}

void AccelEnable(void)
{
    g_bAccelActiv = ENABLE;
}
void AccelDisable(void)
{
    g_bAccelActiv = DISABLE;
}
_Bool AccelState(void)
{
    return g_bAccelActiv;
}

/* Возвращает количество сделанных выстрелов */
int getCountShot(void)
{
    return g_usShot;
}

/* Устанавливает количество сделанных выстрелов */
void setCountShot(int shot)
{
    g_usShot = shot;
}

/* Увеличивает количество сделанных выстрелов */
static void increaseCountShot(void)
{
    g_usShot++;
    if(g_usShot > 9999)
        g_usShot = 0;
    cout("Shot: %d\r\n", g_usShot);
}