
#ifndef _CMD_COMMANDS_H_
#define _CMD_COMMANDS_H_

#include "includes.h"

// extern TEepConfig g_stEepConfig;

typedef struct
{
   size_t adr;   //адрес переменной в озу
   const char* ptrName;   //указатель на текстовую команду
   const char* ptrHelp;   //указатель на краткое описание команды
   int (*pUserParse)(u8* buff, u8* parg, u16 len);   //пользовательская обработка команды
   float limits[2];   //минимальное и максимальное значения параметра
   u32 flags;   //флаги для настройки доступа и формата вывода

#define TYPE_U8 BIT(0)
#define TYPE_S8 BIT(1)
#define TYPE_U16 BIT(2)
#define TYPE_S16 BIT(3)
#define TYPE_U32 BIT(4)
#define TYPE_S32 BIT(5)
#define TYPE_FLOAT BIT(6)
#define NUM_MASK 0x007F

#define OUT_U32 BIT(7)
#define TYPE_STRING BIT(8)

#define LIM BIT(9)   //проверять ли макс и мин. значения
#define CFG_VAL BIT(10)   //сохранять ли в еепром
#define PASS_LVL_0 BIT(11)   //нужен пароль для редактирования переменной
#define PASS_LVL_1 BIT(12)   //нужен пароль для редактирования переменной
#define PASS_LVL_2 BIT(13)
#define SMS_ACCESS_DIS BIT(14)   //запрещено менять через смс
#define READ_DIS BIT(15)   //чтение параметра запрещено
#define READ_PASS BIT(16)   //пароль на чтение параметра
#define ECHO BIT(17)   //посылать ли эхо
#define RAM_VAL BIT(18)   //переменная только в ОЗУ
#define FLAG_OK BIT(19)
#define USER_PROCESSING BIT(20)
#define NOT_SUPPORTED_CMD BIT(21)

#define PASS_EN (PASS_LVL_0 | PASS_LVL_1 | PASS_LVL_2 | READ_PASS)
   u8 uclen;   //размер параметра, для контроля типа TYPE_STRING и записи в еепром

} CMD;

#define SHORT_CMD_RAM(var, name, help, limLOW, limUP, flags) \
   { (size_t)&var, name, help, NULL, limLOW, limUP, flags | RAM_VAL, sizeof(var) },

#define SHORT_CMD(var, name, help, limLOW, limUP, flags) \
   { (size_t)&var, name, help, NULL, limLOW, limUP, flags, sizeof(var) },
#define SHORT_CMD_STR(var, name, help, flags) { (size_t)&var, name, help, NULL, 0, 0, flags, sizeof(var) },

#define SHORT_CMD_USER(name, help, func, flags) { 0, name, help, func, 0, 0, flags, 0 },

#define DEF_FLAGS_U8 (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_U8 | LIM | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_S8 (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_S8 | LIM | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_U16 (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_U16 | LIM | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_S16 (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_S16 | LIM | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_U32 (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_U32 | LIM | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_FL (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_FLOAT | LIM | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_STR (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_STRING | CFG_VAL | FLAG_OK | ECHO)
#define DEF_FLAGS_USER (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | FLAG_OK | ECHO)
#define DEF_FLAGS_NO_OK (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | ECHO)

#define PASS_DEF_FLAGS (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_STRING | CFG_VAL | FLAG_OK)
#define PASS_DEF_FLAGS_U8 (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_U8 | LIM | CFG_VAL | FLAG_OK | ECHO)
#define PASS_DEF_FLAGS_STR (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_STRING | CFG_VAL | FLAG_OK | ECHO)
#define PASS_DEF_FLAGS_STR_USER (PASS_LVL_2 | PASS_LVL_1 | PASS_LVL_0 | TYPE_STRING | CFG_VAL | FLAG_OK | ECHO)

static const CMD cmd_array[] = {

   SHORT_CMD_USER("PASS", "Password entry", pass_entry, PASS_DEF_FLAGS | READ_DIS | USER_PROCESSING)

   // SHORT_CMD(g_stEepConfig.stDevice.eModeDevice, "DEV#MODE",       "0...3",  0, 3,    DEF_FLAGS_U8)
   // SHORT_CMD_USER("RESET",  "Reboot Device",  CmdResetDevice, DEF_FLAGS_USER)
   // SHORT_CMD_RAM(g_stDebugUSART.bGSM_Debug_USART, "D#GSM", "GSM Debug",  0, 1,    DEF_FLAGS_U8)

};

#endif