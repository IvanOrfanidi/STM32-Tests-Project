#ifndef __DISPLAY_H
#define __DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#define DISPLAY_TIMEOUT_BUTTON 15
#define LOW_BAT_TIMEOUT 900
#define SLEEP_TIMEOUT_DEVICE (30 * 60)   // min
#define TIMEOUT_HOLD_BUTTON_WKUP 10

#define TEXT_POSITION_X 38
#define TEXT_POSITION_Y 5

#define SOUND_POSITION_X 52
#define SOUND_POSITION_Y 0

#define BAT_POSITION_X 90
#define BAT_POSITION_Y 8

#define BAT_FAIL_POSITION_X 41
#define BAT_FAIL_POSITION_Y 0

#define DisplayDisable() SSD1306_Disable()

typedef enum
{
   SHOT = 0,
   SOUND,
   RESET_COUNT_SHOT,
   SHUTDOWN,
} tSTEP_DISPLAY;

void vDisplayTask(void* pvParameters);
void msgShutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_H */