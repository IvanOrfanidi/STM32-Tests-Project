#ifndef _PARSER_CMD_H_
#define _PARSER_CMD_H_

#define CMD_ERR_LIM -1
#define CMD_ERR_STR -2
#define CMD_ERR_PASS -3
#define CMD_ERR_ACCESS -4
#define CMD_ERR_TM -5
#define CMD_ERR_EXISTS -6
#define CMD_ERR_CMD -7
#define CMD_ERR_WRNG -8
#define CMD_ERR_CONTINUE -9
#define CMD_ERR_BUSY -10
#define CMD_PASS_OK -11
#define CMD_ERR_TIMEOUT -12
#define CMD_ERR_UNKNOW -13
#define CMD_ERR_MEM -14

#define CMD_NO_ANS -15

#define INTERF_COM_1 0
#define INTERF_COM_USB 1
#define INTERF_SRV 2
#define INTERF_SMS 3

#define ANS_BUFF_SIZE 256

typedef enum
{
   ID_SET_PASS = 192,
} ID_SET_VALUE;

int pass_entry(u8* pansw, u8* parg, u16 len);
int parse_cmd(u8* pcmd, u8* pansw, u8 uc_interf, u16 cmd_sz, u16 answ_sz);
int pass_change(u8* pansw, u8* parg, u16 len);
int findstr(const char* str, char* buf, int size);

void vResetFlagUsbSessionPassOk(void);

#endif