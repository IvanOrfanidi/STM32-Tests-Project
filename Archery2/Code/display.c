
#include "includes.h"

static tSTEP_DISPLAY StepDeviceHandler(_Bool, _Bool, TStatWkupPin);
static void updateScreen(tSTEP_DISPLAY, uint16_t);

static void shotOut(uint16_t);
static void soundOut(uint16_t);
static void resetCountShotOut(uint16_t);
static void shutDown(uint16_t);

static void msgWelcome(uint32_t);
static void sleepMonitoring(portTickType* const, int* const, TStatWkupPin);
static void lowPowerBaMonitoring(portTickType* const, int, float, TStatWkupPin);
static void displayMonitoring(tSTEP_DISPLAY* const, int* const, uint16_t, uint16_t);
static void accelAndUsbMonitoring(TStatWkupPin);
static void buttonMonitoring(tSTEP_DISPLAY* const,
    portTickType* const,
    int* const,
    uint16_t* const,
    uint16_t* const,
    TStatWkupPin);

/* */
void vDisplayTask(void* pvParameters)
{
    static int iDisplayTimeout = DISPLAY_TIMEOUT_BUTTON;
    static portTickType xSleepDeviceTimeout = SLEEP_TIMEOUT_DEVICE;
    tSTEP_DISPLAY eStepDisplay = SHOT;
    uint16_t usResetCountShot = 0;
    uint16_t usDisplayShutdown = 0;

    DISPLAY_PWR_ON;    // Power Display ON

    portTickType xLastWakeTimerDelay = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTimerDelay, (200 / portTICK_RATE_MS));

    portTickType xTimeout = 100;
    xQueueSend(xBuzQueue, &xTimeout, 0);

    /* Display Init */
    SSD1306_Init();
    xLastWakeTimerDelay = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTimerDelay, (200 / portTICK_RATE_MS));

    while(1) {
        float Vbat;
        TStatWkupPin eStatCurrentWkupPin;
        GetBatMeasAndStatWkupPin(&Vbat, &eStatCurrentWkupPin);

        /* Button Monitoring */
        buttonMonitoring(&eStepDisplay,
            &xSleepDeviceTimeout,
            &iDisplayTimeout,
            &usResetCountShot,
            &usDisplayShutdown,
            eStatCurrentWkupPin);

        /* Accelerometer and USB Monitoring */
        accelAndUsbMonitoring(eStatCurrentWkupPin);

        /* Display Monitoring */
        displayMonitoring(&eStepDisplay, &iDisplayTimeout, usResetCountShot, usDisplayShutdown);

        /* Low Power Bat Monitoring */
        lowPowerBaMonitoring(&xSleepDeviceTimeout, iDisplayTimeout, Vbat, eStatCurrentWkupPin);

        /* Sleep Monitor */
        sleepMonitoring(&xSleepDeviceTimeout, &iDisplayTimeout, eStatCurrentWkupPin);

        /* Sleep Action Mode */
        if(!(xSleepDeviceTimeout)) {
            ShutdownDevice();
        }

        xLastWakeTimerDelay = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTimerDelay, (1 / portTICK_RATE_MS));
    }
}

/* Button Monitoring */
static void buttonMonitoring(tSTEP_DISPLAY* const pStepDisplay,
    portTickType* const pSleepDeviceTimeout,
    int* const pDisplayTimeout,
    uint16_t* const pResetCountShot,
    uint16_t* const pDisplayShutdown,
    TStatWkupPin eStatCurrentWkupPin)
{
    static _Bool bStatPreviousBoot = 0;
    static _Bool bStatPreviousSecond = 0;
    static TStatWkupPin eStatPreviousWkupPin = BUT_OFF;
    static uint16_t usTimeoutButPowerOff = TIMEOUT_HOLD_BUTTON_WKUP;
    static _Bool bBuzOnUsbDiscon = FALSE;

    if(eStatCurrentWkupPin != eStatPreviousWkupPin || bStatPreviousBoot != GET_BUT_BOOT ||
        bStatPreviousSecond != GET_BUT_SECOND) {
        eStatPreviousWkupPin = eStatCurrentWkupPin;
        bStatPreviousBoot = GET_BUT_BOOT;
        bStatPreviousSecond = GET_BUT_SECOND;
        usTimeoutButPowerOff = TIMEOUT_HOLD_BUTTON_WKUP;
        *pDisplayShutdown = 0;

        if(*pStepDisplay == SHUTDOWN) {
            *pStepDisplay = SHOT;
        }

        if(GET_BUT_BOOT || GET_BUT_SECOND || eStatCurrentWkupPin == BUT_ON || eStatCurrentWkupPin == USB_CONNECT) {
            *pSleepDeviceTimeout = SLEEP_TIMEOUT_DEVICE;
            *pDisplayTimeout = DISPLAY_TIMEOUT_BUTTON;
            portTickType xTimeout = 50;
            xQueueSend(xBuzQueue, &xTimeout, 0);
            if(eStatCurrentWkupPin == USB_CONNECT) {
                bBuzOnUsbDiscon = TRUE;
            }

            /* Delay */
            uint8_t timeout = 50;
            while(timeout--) {
                if(bStatPreviousBoot != GET_BUT_BOOT || bStatPreviousSecond != GET_BUT_SECOND) {
                    break;
                }
                portTickType xLastWakeTimerDelay = xTaskGetTickCount();
                vTaskDelayUntil(&xLastWakeTimerDelay, (10 / portTICK_RATE_MS));
            }

            *pStepDisplay = StepDeviceHandler(bStatPreviousBoot, bStatPreviousSecond, eStatCurrentWkupPin);
            if(bBuzOnUsbDiscon) {
                *pStepDisplay = SHOT;
            }
        }
    }
    else {
        if((eStatCurrentWkupPin == BUT_ON) && (!(bStatPreviousBoot)) && (!(bStatPreviousSecond))) {
            if(usTimeoutButPowerOff) {
                usTimeoutButPowerOff--;
            }
            else {
                *pStepDisplay = SHUTDOWN;
                *pDisplayTimeout = DISPLAY_TIMEOUT_BUTTON;
                *pDisplayShutdown += 16;
            }

            if(*pDisplayShutdown >= 128) {
                *pSleepDeviceTimeout = 0;
            }
        }
        if((eStatCurrentWkupPin != BUT_ON) && (bStatPreviousBoot) && (!(bStatPreviousSecond))) {
            *pStepDisplay = RESET_COUNT_SHOT;
            if(getCountShot()) {
                (*pResetCountShot) += 16;
            }

            if(*pResetCountShot >= 128) {
                *pResetCountShot = 0;
                setCountShot(0);
                *pStepDisplay = SHOT;
            }
        }
        else {
            *pResetCountShot = 0;
        }
    }

    if((*pStepDisplay == RESET_COUNT_SHOT) && (!(*pResetCountShot))) {
        *pStepDisplay = SHOT;
    }

    if(bBuzOnUsbDiscon && eStatCurrentWkupPin != USB_CONNECT) {
        bBuzOnUsbDiscon = FALSE;
        *pDisplayTimeout = DISPLAY_TIMEOUT_BUTTON;
        portTickType xTimeout = 50;
        xQueueSend(xBuzQueue, &xTimeout, 0);
    }
}

/* Accelerometer and USB Monitoring */
static void accelAndUsbMonitoring(TStatWkupPin eStatCurrentWkupPin)
{
    if(eStatCurrentWkupPin == USB_CONNECT) {
#ifdef __USE_DEBUG_USB__    // if Debug in USB Enable.
        AccelEnable();
#else
        AccelDisable();

        /* Configure USB Driver */
        usbInitDriver();
#endif
    }
    else {
        AccelEnable();
#ifndef __USE_DEBUG_USB__    // If Debug in USB Disable.
        usbDeInitDriver();
#endif
    }
}

/* Display Monitor */
static void displayMonitoring(tSTEP_DISPLAY* const pStepDisplay,
    int* const pDisplayTimeout,
    uint16_t usResetCountShot,
    uint16_t usDisplayShutdown)
{
    static _Bool bWelcome = TRUE;
    if(*pDisplayTimeout) {
        SSD1306_Enable();
        SSD1306_Fill(SSD1306_COLOR_BLACK);
        if(bWelcome) {
            msgWelcome(2000);
            bWelcome = FALSE;
        }
        else {
            uint16_t temp;
            switch(*pStepDisplay) {
                case SHOT:
                    temp = getCountShot();
                    break;
                case SOUND:
                    temp = GetSound();
                    break;
                case RESET_COUNT_SHOT:
                    temp = usResetCountShot;
                    break;
                case SHUTDOWN:
                    temp = usDisplayShutdown;
                    break;
            }
            updateScreen(*pStepDisplay, temp);    // Update Screen
        }
        (*pDisplayTimeout)--;
    }
    else {
        *pStepDisplay = SHOT;    // Reset type display msg
        DisplayDisable();
        portTickType xLastWakeTimerDelay = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTimerDelay, (10 / portTICK_RATE_MS));
    }
}

/* Low Power Bat Monitoring */
static void lowPowerBaMonitoring(portTickType* const pSleepDeviceTimeout,
    int iDisplayTimeout,
    float Vbat,
    TStatWkupPin eStatCurrentWkupPin)
{
    static portTickType xTimPreviousBuzLowPwr = 0;
    static uint16_t iTimeoutLowBat = (LOW_BAT_TIMEOUT - LOW_BAT_TIMEOUT / 3);
    static portTickType WakeLowBat = 0;

    if(Vbat < VBAT_LOW && eStatCurrentWkupPin != USB_CONNECT) {
        if(xTaskGetTickCount() >= WakeLowBat) {
            WakeLowBat += configTICK_RATE_HZ;
            if(iTimeoutLowBat) {
                iTimeoutLowBat--;
            }
        }
        if(!(iTimeoutLowBat)) {
            *pSleepDeviceTimeout = 0;
        }
    }
    else {
        iTimeoutLowBat = LOW_BAT_TIMEOUT;
    }

    /* Индикация разряженной батарейки, периодически мигаем */
    if(iTimeoutLowBat < (LOW_BAT_TIMEOUT - LOW_BAT_TIMEOUT / 3) && eStatCurrentWkupPin != USB_CONNECT) {    //РЈСЂРѕРІРµРЅСЊ РїСЂРё РєРѕС‚РѕСЂРѕРј Р±СѓРґРµРј РёРЅС„РѕСЂРјРёСЂРѕРІР°С‚СЊ Рѕ РЅРёР·РєРѕРј Р·Р°СЂСЏРґРµ РђРљР‘.
        portTickType xLastWakeTimerDelay = xTaskGetTickCount();
        if(xLastWakeTimerDelay > xTimPreviousBuzLowPwr) {
            xTimPreviousBuzLowPwr = xLastWakeTimerDelay + 10000;

            portTickType xTimeout = 100;
            xQueueSend(xBuzQueue, &xTimeout, 0);
            xLastWakeTimerDelay = xTaskGetTickCount();
            vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));
            xTimeout = 100;
            xQueueSend(xBuzQueue, &xTimeout, 0);

            xLastWakeTimerDelay = xTaskGetTickCount();
            vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));

            if(!(iDisplayTimeout)) {
                ssd1306_I2C_FillDisplay(
                    BAT_FAIL_POSITION_X, BAT_FAIL_POSITION_Y, ImgBatFail.ImgHeight, ImgBatFail.ImgWidth, ImgBatFail.pData);
                SSD1306_UpdateScreen();
            }

            xLastWakeTimerDelay = xTaskGetTickCount();
            vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));

            if(!(iDisplayTimeout)) {
                DisplayDisable();
                xLastWakeTimerDelay = xTaskGetTickCount();
                vTaskDelayUntil(&xLastWakeTimerDelay, (10 / portTICK_RATE_MS));
            }
        }
    }
}

/* Sleep Monitor */
static void sleepMonitoring(portTickType* const pSleepDeviceTimeout,
    int* const pDisplayTimeout,
    TStatWkupPin eStatCurrentWkupPin)
{
    static uint16_t previous_shot = 0;
    static portTickType WakeTickSleep = 0;
    int curent_shot = getCountShot();
    if((curent_shot != previous_shot && (*pSleepDeviceTimeout)) || (eStatCurrentWkupPin == USB_CONNECT)) {
        if(curent_shot != previous_shot) {
            /* Пищим зуммером при выстреле */
            portTickType xTimeout = 100;
            xQueueSend(xBuzQueue, &xTimeout, 0);

            /* Ставим небольшую задержку на вывод данных на дисплей, так как лук должен упасть */
            portTickType xLastWakeTimerDelay = xTaskGetTickCount();
            vTaskDelayUntil(&xLastWakeTimerDelay, (400 / portTICK_RATE_MS));

            *pDisplayTimeout = DISPLAY_TIMEOUT_BUTTON;
            previous_shot = curent_shot;    //Обновляем старые показания
        }

        *pSleepDeviceTimeout = SLEEP_TIMEOUT_DEVICE;
    }
    else {
        if(xTaskGetTickCount() >= WakeTickSleep) {
            WakeTickSleep = xTaskGetTickCount() + configTICK_RATE_HZ;
            (*pSleepDeviceTimeout)--;    // Decrement Timeout
        }
    }
}

static void updateScreen(tSTEP_DISPLAY eStepDisplay, uint16_t value)
{
    void (*ptrDisplayOut[])(uint16_t value) = { shotOut, soundOut, resetCountShotOut, shutDown };

    vTaskSuspendAll();
    (*ptrDisplayOut[eStepDisplay])(value);
    SSD1306_UpdateScreen();

    portTickType xLastWakeTimerDelay = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTimerDelay, (200 / portTICK_RATE_MS));
    xTaskResumeAll();
}

static tSTEP_DISPLAY StepDeviceHandler(_Bool bStatPreviousBoot,
    _Bool bStatPreviousSecond,
    TStatWkupPin eStatCurrentWkupPin)
{
    static tSTEP_DISPLAY eStepDisplay = SHOT;
    if((bStatPreviousBoot) && (!(bStatPreviousSecond)) && (eStatCurrentWkupPin != BUT_ON)) {
        if(getCountShot()) {
            eStepDisplay = RESET_COUNT_SHOT;
        }
        else {
            eStepDisplay = SHOT;
        }
    }
    if((!(bStatPreviousBoot)) && (bStatPreviousSecond) && (eStatCurrentWkupPin != BUT_ON)) {
        if(eStepDisplay == SOUND) {
            _Bool sound = GetSound();
            sound ^= ENABLE;
            SetSound(sound);
        }
        eStepDisplay = SOUND;
    }
    if((!(bStatPreviousBoot)) && (!(bStatPreviousSecond)) && (eStatCurrentWkupPin == BUT_ON)) {
        eStepDisplay = SHOT;
    }
    return eStepDisplay;
}

static void shotOut(uint16_t usCurrentShot)
{
    TStatWkupPin eStatCurrentWkupPin = BUT_OFF;
    float Vbat = 0;
    GetBatMeasAndStatWkupPin(&Vbat, &eStatCurrentWkupPin);
    if(eStatCurrentWkupPin == USB_CONNECT) {
        ssd1306_I2C_FillDisplay(
            BAT_POSITION_X, BAT_POSITION_Y, ImgBatCharge.ImgHeight, ImgBatCharge.ImgWidth, ImgBatCharge.pData);
    }
    else {
        if(Vbat > VBAT_FULL) {
            ssd1306_I2C_FillDisplay(
                BAT_POSITION_X, BAT_POSITION_Y, ImgBatFull.ImgHeight, ImgBatFull.ImgWidth, ImgBatFull.pData);
        }
        else if(Vbat >= VBAT_HIGH && Vbat < VBAT_FULL) {
            ssd1306_I2C_FillDisplay(
                BAT_POSITION_X, BAT_POSITION_Y, ImgBatHigh.ImgHeight, ImgBatHigh.ImgWidth, ImgBatHigh.pData);
        }
        else if(Vbat >= VBAT_MEDIUM && Vbat < VBAT_HIGH) {
            ssd1306_I2C_FillDisplay(
                BAT_POSITION_X, BAT_POSITION_Y, ImgBatMedium.ImgHeight, ImgBatMedium.ImgWidth, ImgBatMedium.pData);
        }
        else if(Vbat >= VBAT_LOW && Vbat < VBAT_MEDIUM) {
            ssd1306_I2C_FillDisplay(
                BAT_POSITION_X, BAT_POSITION_Y, ImgBatLow.ImgHeight, ImgBatLow.ImgWidth, ImgBatLow.pData);
        }
        else if(Vbat < VBAT_LOW) {
            ssd1306_I2C_FillDisplay(
                BAT_POSITION_X, BAT_POSITION_Y, ImgBatNone.ImgHeight, ImgBatNone.ImgWidth, ImgBatNone.pData);
        }
    }

    uint16_t text_pos_x = TEXT_POSITION_X;
    uint16_t text_pos_y = TEXT_POSITION_Y;

    if(usCurrentShot > 10) {
        text_pos_x = TEXT_POSITION_X - 5;
    }
    if(usCurrentShot > 100) {
        text_pos_x = TEXT_POSITION_X - 15;
    }
    if(usCurrentShot > 1000) {
        text_pos_x = TEXT_POSITION_X - 25;
    };

    char strMsg[32] = { 0 };
    sprintf(strMsg, "%d", usCurrentShot);
    cout("Shot: %s", strMsg);
    SSD1306_GotoXY(text_pos_x, text_pos_y);
    SSD1306_Puts(strMsg, &Font_16x26, SSD1306_COLOR_WHITE);    // put string
}

// GetSound()
static void soundOut(uint16_t sound)
{
    if(sound) {
        ssd1306_I2C_FillDisplay(
            SOUND_POSITION_X, SOUND_POSITION_Y, ImgSoundOn.ImgHeight, ImgSoundOn.ImgWidth, ImgSoundOn.pData);
    }
    else {
        ssd1306_I2C_FillDisplay(
            SOUND_POSITION_X, SOUND_POSITION_Y, ImgSoundOff.ImgHeight, ImgSoundOff.ImgWidth, ImgSoundOff.pData);
    }
}

static void resetCountShotOut(uint16_t ucResetCountShot)
{
    char strMsg[48];
    sprintf(strMsg, "Reset Count");
    SSD1306_GotoXY(4, 0);
    SSD1306_Puts(strMsg, &Font_11x18, SSD1306_COLOR_WHITE);    // put string

    strcpy(strMsg, "------------->>YES");
    SSD1306_GotoXY(0, 21);
    SSD1306_Puts(strMsg, &Font_7x10, SSD1306_COLOR_WHITE);    // put string

    if(ucResetCountShot) {
        SSD1306_DrawFilledRectangle(0, 20, ucResetCountShot, 20, SSD1306_COLOR_WHITE);
    }
}

static void shutDown(uint16_t usDisplayShutdown)
{
    char strMsg[48];
    sprintf(strMsg, "Shutdown");
    SSD1306_GotoXY(20, 0);
    SSD1306_Puts(strMsg, &Font_11x18, SSD1306_COLOR_WHITE);    // put string

    strcpy(strMsg, "------------->>YES");
    SSD1306_GotoXY(0, 21);
    SSD1306_Puts(strMsg, &Font_7x10, SSD1306_COLOR_WHITE);    // put string

    if(usDisplayShutdown) {
        SSD1306_DrawFilledRectangle(0, 20, usDisplayShutdown, 20, SSD1306_COLOR_WHITE);
    }
}

static void msgWelcome(uint32_t uiWakeTimeMsgWelcome)
{
    uint16_t count_img_wel = GetImageWelcome();    // Получаем счетчик картинок преветствия

    ssd1306_I2C_FillDisplay(0, 0, SSD1306_HEIGHT, SSD1306_WIDTH, ptrArcheryWel[count_img_wel]);
    SSD1306_UpdateScreen();

    /* перебираем картинки */
    count_img_wel++;
    if(count_img_wel >= SIZE_ARR_IMG_WEL)
        count_img_wel = 0;
    SetImageWelcome(count_img_wel);

    portTickType xLastWakeTimerDelay = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTimerDelay, ((TickType_t)uiWakeTimeMsgWelcome / portTICK_RATE_MS));
}

void msgShutdown(void)
{
    SSD1306_Enable();

    if(GetImageWelcome() & 1) {
        SSD1306_ToggleInvert();    // Через раз влючаем инверсию картинки.
    }

    ssd1306_I2C_FillDisplay(0, 0, ImgShutdown.ImgHeight, ImgShutdown.ImgWidth, ImgShutdown.pData);
    SSD1306_UpdateScreen();

    portTickType xTimeout = 100;
    xQueueSend(xBuzQueue, &xTimeout, 0);

    portTickType xLastWakeTimerDelay = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTimerDelay, (250 / portTICK_RATE_MS));

    xTimeout = 100;
    xQueueSend(xBuzQueue, &xTimeout, 0);

    xLastWakeTimerDelay = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTimerDelay, (2000 / portTICK_RATE_MS));
}
