
#ifndef ESP8266_H_
#define ESP8266_H_

#define CR '\r'
#define LF '\n'
#define CR_LF "\r\n"

/* at commands */
#define AT "AT"
#define AT_GMR "AT+GMR"
#define AT_RST "AT+RST"
#define ATE0 "ATE0"
#define ATE1 "ATE1"
#define AT_CWSAP "AT+CWSAP="
#define AT_CWJAP "AT+CWJAP="
#define AT_CIFSR "AT+CIFSR"
#define AT_CIPSEND "AT+CIPSEND="
#define AT_CIPSTART "AT+CIPSTART="
#define AT_CIPCLOSE "AT+CIPCLOSE="
#define AT_CIOBAUD "AT+CIOBAUD="

#define ESP_MODE_STA "AT+CWMODE=1"
#define ESP_MODE_AP "AT+CWMODE=2"
#define ESP_MODE_BOTH "AT+CWMODE=3"

#define ESP_SINGLE_CONNECTION "AT+CIPMUX=0"
#define ESP_MULTIPE_CONNECTION "AT+CIPMUX=1"

#define ESP_START_SERVER "AT+CIPSERVER=1"

typedef enum
{
   WAIT_DATA = -128,
   ERR_SOCKET = -9,
   ERR_SEND_DATA = -8,
   ERR_NO_PROMPT = -7,
   ERR_OVERFLOW_ANS = -6,
   ERR_LF_ANS = -5,
   ERR_SIZE_BUF = -4,
   ERR_UNK_ANS = -3,
   ERR_TIMEOUT = -2,
   ERR_ECHO = -1,
   RET_SEND_OK = 0,
   RET_ERROR = 1,
   RET_CONNECT = 2,
   RET_RST = 3,
   RET_IPD = 4,
   RET_CLOSE = 5,
   RET_PROMPT = 6,
   RET_OK = 7,
   RET_busy = 8,
   RET_WIFI_GOT_IP = 9,
} ANS_INFO;

#define MAX_PARAMETRS_RESPONSES 2

typedef struct
{
   ANS_INFO eResType;
   uint8_t ucCountParamRes;
   int iValueParamRes[MAX_PARAMETRS_RESPONSES];
   _Bool bEcho;
} ESP_INFO;

void esp_write(const char* pBuf, uint16_t size);
int esp_read(char* pBuf, uint16_t iMaxLen);
ANS_INFO cmd_send(const char* pCmd, ESP_INFO* pstEsp, char* pRet, int iSize);
ANS_INFO esp_parser(ESP_INFO* pstEsp, const char* pEcho, char* pRet, int iSize);

#endif