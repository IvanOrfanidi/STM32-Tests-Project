#include <string.h>

#include "includes.h"
#include "gsm_parser.h"

#define DEBUG_PARSER 0
char strTempPaswd[10];

extern unsigned char g_au8RxBufferUSART2[];
unsigned char* pRxBufferGSM = g_au8RxBufferUSART2;

char parser_ret;
char parser_byte;
char parser_j;
char parser_n = 0;
char start = 0;
char parser_row;
char parser_col = 0;
char v_sign = 0;
char s_copy = 0;
char p_copy = 1;
int parser_size = 0;
int parser_offset = 0;
int parser_state = 0;
u8 parser_mask = 0;

const MSG_TABLE t_msg[M_TOTAL] = {
   { 2, 0, 0x00, "OK" },   //  1) OK
   { 5, 0, 0x00, "ERROR" },   //  2) ERROR
   { 5, 4, 0x08, "^SIS:" },   //  3)
   { 6, 2, 0x00, "^SISR:" },   //  4) ^SISR: 0, 1
   { 6, 3, 0x00, "^SISW:" },   //  5) ^SISW: 0, 1   ^SISW: 0,512,123
   { 6, 6, 0x00, "^SISI:" },   //  6) ^SISI: 0,2,0,0,0,0
   { 6, 4, 0x08, "^SICI:" },   //  7) ^SICI: 0,2,1,"10.153.65.148"
   { 9, 0, 0x00, "^SYSSTART" },   //  8) ^SYSSTART
   { 13, 0, 0x00, "^SMSO: MS OFF" },   //  9) ^SMSO: MS OFF
   { 6, 2, 0x00, "^SCKS:" },   // 10) ^SCKS: 0,1
   { 6, 3, 0x00, "^SCTM:" },   // 11) ^SCTM: 0,0,25
   { 6, 1, 0x01, "^SCID:" },   // 12) ^SCID: 89701012417725707318
   { 9, 0, 0x00, "^SHUTDOWN" },   // 13) ^SHUTDOWN
   { 5, 2, 0x00, "+CSQ:" },   // 14) +CSQ: 31,99
   { 14, 0, 0x00, "+CPIN: SIM PIN" },   // 15) +CPIN: SIM PIN
   { 14, 0, 0x00, "+CPIN: SIM PUK" },   // 16) +CPIN: SIM PUK
   { 12, 0, 0x00, "+CPIN: READY" },   // 17) +CPIN: READY
   { 6, 2, 0x00, "+CREG:" },   // 18) +CREG: 0,1
   { 6, 3, 0x00, "+COPS:" },   // 19) +COPS: 0,2,"25001"   +COPS: (2,"MTS-RUS",,"25001"),(3,"Bee
                               // Line",,"25099"),(3,"MOTIV",,"25020"),(3,"MegaFon",,"25002"),,(0-4),(0,2)
   { 12, 1, 0x00, "+CMTI: \"ME\"," },   // 20) +CMTI: "MT",1
   { 12, 1, 0x00, "+CMTI: \"MT\"," },   // 21) +CMTI: "MT",1
   { 12, 1, 0x00, "+CMTI: \"SM\"," },   // 22) +CMTI: "MT",1
   { 11, 0, 0x00, "+CME ERROR:" },   // 23)
   { 6, 4, 0x00, "^SMGL:" },   // 24) ^SMGL: 1,1,,93
   { 6, 3, 0x00, "^SMGR:" },   // 25) ^SMGR: 1,,33
   { 4, 0, 0x00, "RING" },   // 26) RING
   { 2, 0, 0x00, "> " },   // 27) "> "
   { 18, 0, 0x00, "^SBC: Undervoltage" },   // 28) ^SBC: Undervoltage - питающее напряжение ME близко к нижнему порогу
   { 17, 0, 0x00, "^SBC: Overvoltage" },   // 29) ^SBC: Overvoltage  - питающее напряжение ME близко к верхнему порогу
   { 7,
     0,
     0x00,
     "^SMOND:" },   // 30)
                    // ^SMOND:250,01,00EA,0AE1,23,756,47,,,0,,,0,250,01,00EA,0ADE,40,1022,47,250,01,00DD,0A5E,23,783,78,250,01,00EA,0AE2,43,795,78,250,01,00EA,0AE0,26,791,78,250,01,00EA,4EA7,20,748,86,250,01,00EA,,20,669,87,1,31,99
   { 12, 4, 0x02, "^SIND: nitz," },   // 31) ^SIND: nitz,0,"13/03/29,07:54:49",+16,0
   { 7, 0, 0x00, "ABORTED" },   // 32) ABORTED
   { 6, 3, 0x07, "+CUSD:" },   // 33) CUSD
   { 7, 0, 0x00, "NO CARRIER" },   // 34) NO CARRIER
   { 6, 1, 0x00, "^SPIC:" },   // 35) SPIC
   { 6, 4, 0x00, "+CMGL:" },   // 36) +CMGL: 1,1,,93(SIM800)
   { 7, 1, 0x00, "+CGATT:" },   // 37) +CGATT: 1
   { 12, 0, 0x00, "0, CONNECT OK" },   // 38) CONNECT OK
   { 12, 0, 0x00, "1, CONNECT OK" },   // 39) CONNECT OK
   { 10, 0, 0x00, "0, SEND OK" },   // 40) SEND OK
   { 10, 0, 0x00, "1, SEND OK" },   // 41) SEND OK
   { 8, 0, 0x00, "+RECEIVE" },   // 42) +RECEIVE
   { 11, 0, 0x00, "0, CLOSE OK" },   // 43) CLOSE OK
   { 11, 0, 0x00, "1, CLOSE OK" },   // 44) CLOSE OK
   { 9, 0, 0x00, "0, CLOSED" },   // 45) CLOSED
   { 9, 0, 0x00, "1, CLOSED" },   // 46) CLOSED
   { 8, 2, 0x00, "+FTPGET:" },   // 47)+FTPGET
   { 17, 2, 0x00, "+SJDR:" },   // 48)+SJDR:
   { 12, 3, 0x00, "+HTTPACTION:" },   // 49)+HTTPACTION:
   { 10, 1, 0x00, "+HTTPREAD:" },   // 50)+HTTPREAD: <date_len>
   { 11,
     3,
     0x00,
     "+BTPAIRING:" },   // 51)+BTPAIRING: <name>,<address>,<passcode> (+BTPAIRING: "LG-E612",e8:92:a4:61:bc:65,088094)
   { 14,
     2,
     0x00,
     "+BTCONNECTING:" },   // 52)+BTCONNECTING: <address>, <profile name> (+BTCONNECTING: "e8:92:a4:61:bc:65","SPP")
   { 11,
     2,
     0x00,
     "+BTCONNECT:" },   // 53) +BTCONNECT: <id>,<name>,<address>,<profile name> (+BTCONNECT:
                        // 2,"LG-E612",e8:92:a4:61:bc:65,"SPP"  ||  +BTCONNECT: 1,"LG-E612",e8:92:a4:61:bc:65,"HFP")
   { 11, 2, 0x00, "+BTSPPDATA:" },   // 54) +BTSPPDATA: 1,8,qwerty
   { 3, 0, 0x00, "RDY" },   // 55) RDY
   { 11, 0, 0x00, "+BTDISCONN:" },   // 54) +BTDISCONN: "LG-E612",e8:92:a4:61:bc:65,"SPP"
   { 10, 0, 0x00, "Call Ready" },   // 55) Call Ready
};

u32 calculate(u8 sign, u8 base, u8 parser_byte, u32 res)
{
   if (sign == 0)
   {
      switch (base)
      {
      case 16:
         if (parser_byte >= 'A' && parser_byte <= 'F')
         {
            res *= base;
            res += (parser_byte - 'A' + 10);
            break;
         }
         if (parser_byte >= 'a' && parser_byte <= 'f')
         {
            res *= base;
            res += (parser_byte - 'a' + 10);
            break;
         }
      case 10:
         if (parser_byte >= '0' && parser_byte <= '9')
         {
            res *= base;
            res += (parser_byte - '0');
            break;
         }
         // res = 0;
         break;
      }
   }
   else
   {
      switch (base)
      {
      case 16:
         if (parser_byte >= 'A' && parser_byte <= 'F')
         {
            res *= base;
            res -= (parser_byte - 'A' + 10);
            break;
         }
         if (parser_byte >= 'a' && parser_byte <= 'f')
         {
            res *= base;
            res -= (parser_byte - 'a' + 10);
            break;
         }
      case 10:
         if (parser_byte >= '0' && parser_byte <= '9')
         {
            res *= base;
            res -= (parser_byte - '0');
            break;
         }
         // res = 0;
         break;
      }
   }

   return res;
}

void parser_smond(GSM_INFO* a, int* state, u8* parser_mask, char parser_byte)
{
   if (parser_byte == ',')
   {
      (*state)++;
      if ((*parser_mask == 0 && *state == 13) || (*parser_mask > 0 && *parser_mask < 7 && *state == 7))
      {
         *state = 0;
         if (a->inf->station[a->count].mcc != 0 && a->inf->station[a->count].mnc != 0 &&
             a->inf->station[a->count].lac != 0 && a->inf->station[a->count].cell != 0 &&
             a->inf->station[a->count].rxlev != 0)
         {
            a->count++;
         }
         else
         {
            a->inf->station[a->count].mcc = 0;
            a->inf->station[a->count].mnc = 0;
            a->inf->station[a->count].lac = 0;
            a->inf->station[a->count].cell = 0;
            a->inf->station[a->count].rxlev = 0;
         }
         (*parser_mask)++;
      }
   }
   else if (*parser_mask < 7)
   {
      switch (*state)
      {
      case 0:
         a->inf->station[a->count].mcc = calculate(0, 10, parser_byte, a->inf->station[a->count].mcc);
         break;
      case 1:
         a->inf->station[a->count].mnc = calculate(0, 10, parser_byte, a->inf->station[a->count].mnc);
         break;
      case 2:
         a->inf->station[a->count].lac = calculate(0, 16, parser_byte, a->inf->station[a->count].lac);
         break;
      case 3:
         a->inf->station[a->count].cell = calculate(0, 16, parser_byte, a->inf->station[a->count].cell);
         break;
      case 6:
         a->inf->station[a->count].rxlev = calculate(0, 10, parser_byte, a->inf->station[a->count].rxlev);
         break;
      }
   }
   else
   {
      if (*state == 0)
      {
         a->inf->ta = calculate(0, 10, parser_byte, a->inf->ta);
      }
   }
}

void parser_cops(GSM_INFO* a, int* state, char parser_byte)
{
   // разбираем параметры сообщения
   switch (*state)
   {
   case 0:
      if (parser_byte == '(')
         *state = 1;   // параметры команды at+cops=?
      else if (parser_byte == ',')
         *state = 6;
      else if (parser_byte >= '0' && parser_byte <= '9' && a->count == 0)
         *state = 7;   // параметры команды at+cops?
      break;
   case 1:
      if (parser_byte < '0' || parser_byte > '9')
      {
         *state = 0;
         break;
      }
      a->ops[a->count].status = parser_byte - '0';
      *state = 2;
      break;
   case 2:
   case 3:
   case 4:
      if (parser_byte == ',')
         (*state)++;
      break;
   case 5:
      if (parser_byte != ',')
      {
         a->ops[a->count].opName = calculate(0, 10, parser_byte, a->ops[a->count].opName);
      }
      else
      {
         a->count++;
         *state = 0;
      }
      break;
   }
}

void parser_sind_nitz(GSM_INFO* a, char* v_sign, char parser_byte)
{
   if (parser_byte == ',' || parser_byte == '/' || parser_byte == ':')
   {
      *v_sign = 0;
      a->count++;
   }
   else if (a->count > 0)
   {
      if (parser_byte != '-')
      {
         a->msg[a->count - 1].var = calculate(*v_sign, 10, parser_byte, a->msg[a->count - 1].var);
      }
      else
      {
         *v_sign = -1;
      }
   }
}

// Возвращает длину //
int gsm_parser(const char* cmd, GSM_INFO* a, char* buf, int b_size, uint32_t wait)
{
   parser_j = 0;
   parser_n = 0;
   start = 0;
   parser_row = M_OK;
   parser_col = 0;
   v_sign = 0;
   s_copy = 0;
   p_copy = 1;
   parser_size = 0;
   parser_offset = 0;
   parser_state = 0;
   parser_mask = 0;
#ifdef USE_BLUETOOTH
   uint8_t ucSymTrue = 0;
#endif
   a->m_type = 0xFF;
   isEndWaitPeriod(100);

   // DS_GSM("GSM__ ", pRxBufferGSM);

   for (;;)
   {
      osDelay(1);
      parser_ret = gsm_read(&parser_byte, 1, wait);

      if (GSM_STATUS_ON == 0)
      {
         // GSM модуль выключился во время работы
         a->m_type = M_SYSSTART;
         break;
      }

      if (parser_ret != 1)
      {
         if (parser_col != 0)
         {
            // конец данных
            a->count += parser_n;
            buf[parser_offset] = 0;

            if (parser_col == t_msg[parser_row].m_size)
            {
               if (parser_row == M_RING)
               {
                  // g_bfRing = 1;
                  return 0;
               }
               if (parser_row == M_NO_CARRIER)
               {
                  // g_bfRing = 0;
                  return 0;
               }
               a->m_type = parser_row;
            }

            DP_GSM("\r");
         }
         break;
      }
      else
      {
         if (parser_byte > 0x1F && parser_byte < 0x7F)
         {
            DP_GSM("%c", parser_byte);
         }
         else
         {
            if (parser_byte == '\r')
            {
               DP_GSM(" \\r");
            }
            else
            {
               if (parser_byte == '\n')
               {
                  DP_GSM(" \\n");
               }
               else
               {
                  DP_GSM(".");
               }
            }
         }
      }
      parser_size++;

      if (parser_byte == '\r')
      {
         if ((start == 0 && (parser_offset == 0 || p_copy == 0)) || (start == 1 && parser_row == M_ECHO))
         {
            // начало данных
            memset(a, 0, sizeof(GSM_INFO));
            if (parser_row == M_ECHO)
            {
               // принято эхо на команду
               a->m_echo = 1;

               DP_GSM("\r");
            }

            parser_n = 0;
            parser_row = M_OK;
            parser_col = 0;
            parser_mask = 0;
            parser_offset = 0;
            v_sign = 0;
            s_copy = 0;
            p_copy = 1;
            a->m_type = 0xFF;
            start = 1;
         }
         continue;
      }
      else if (parser_byte == '\n')
      {
         if (start == 0)
         {
            if (parser_offset != 0 && p_copy != 0)
            {
               // конец данных, без идентификатора
               buf[parser_offset] = 0;
               a->m_type = M_STRING;
               a->msg[0].str = buf;
               a->msg[0].size = parser_offset;
               a->count = 1;

               DP_GSM("\r");
               return parser_size;
            }
         }
         else if (parser_col != 0)
         {
            // конец данных
            a->count += parser_n;
            buf[parser_offset] = 0;

            if (parser_col == t_msg[parser_row].m_size)
            {
               /*
               if(parser_row == M_UVOLTAGE || parser_row == M_OVOLTAGE) {
                   // Обнаружены проблемы с питанием
                   Throw(ERR_POWER);
               }
               */
               a->m_type = parser_row;
               if (parser_row == M_RING)
               {
                  // g_bfRing = 1;
                  return 0;
               }
               if (parser_row == M_NO_CARRIER)
               {
                  // g_bfRing = 0;
                  return 0;
               }
            }

            DP_GSM("\r");
            return parser_size;
         }
         continue;
      }
      else
      {
         if ((parser_col == 0 || parser_byte == '^') && start == 0)
         {
            // начало данных
            parser_n = 0;
            parser_row = M_OK;
            parser_col = 0;
            parser_mask = 0;
            parser_offset = 0;
            v_sign = 0;
            s_copy = 0;
            p_copy = 1;
            memset(a, 0, sizeof(GSM_INFO));
            a->m_type = 0xFF;
            start = 1;
         }
      }

      if (start != 0)
      {
         if (parser_col == 0)
         {
            // определение первого символа в заголовке сообщения от GSM модуля
            if (parser_byte != t_msg[parser_row].msg[parser_col])
            {
               for (parser_j = parser_row + 1; parser_j < M_TOTAL; parser_j++)
               {
                  if (parser_byte == t_msg[parser_j].msg[parser_col])
                  {
                     parser_row = parser_j;
                     break;
                  }
               }
               if (parser_j == M_TOTAL)
               {
                  if (cmd != 0 && *cmd == parser_byte)
                  {
                     // принято эхо на команду
                     parser_row = M_ECHO;
                  }
                  else
                  {
                     start = 0;
                  }
               }
            }
            parser_col++;
         }
         else
         {
            switch (parser_row)
            {
            case M_OK:
            case M_CME_ERROR:
            case M_ERROR:
            case M_ABORTED:
            case M_SHUTDOWN:
            case M_SYSSTART:
            case M_RDY:
            case M_SMSO:
            case M_SIM_PIN:
            case M_SIM_PUK:
            case M_SIM_READY:
            case M_RING:
            case M_PROMPT:
               // определение заголовка без параметров
               if (parser_col < t_msg[parser_row].m_size)
               {
                  if (parser_byte != t_msg[parser_row].msg[parser_col])
                  {
                     for (parser_j = parser_row + 1; parser_j < M_TOTAL; parser_j++)
                     {
                        if (parser_byte == t_msg[parser_j].msg[parser_col])
                        {
                           parser_row = parser_j;
                           break;
                        }
                     }
                     if (parser_j == M_TOTAL)
                     {
                        start = 0;
                     }
                  }
                  parser_col++;
               }
               break;
            default:
               // определение заголовка с параметрами
               if (parser_col < t_msg[parser_row].m_size)
               {
                  if (parser_byte != t_msg[parser_row].msg[parser_col])
                  {
                     for (parser_j = parser_row + 1; parser_j < M_TOTAL; parser_j++)
                     {
                        if (parser_byte == t_msg[parser_j].msg[parser_col])
                        {
                           parser_row = parser_j;
                           break;
                        }
                     }
                     if (parser_j == M_TOTAL)
                     {
                        start = 0;
                     }
                  }
                  parser_col++;
               }
               else
               {
                  p_copy = 0;

                  switch (parser_row)
                  {
                  case M_ECHO:
                     if (parser_byte == cmd[parser_col])
                     {
                        parser_col++;
                     }
                     else
                     {
                        start = 0;
                     }
                     break;
                  case M_SMOND:
                     a->inf = &g_base_station;
                     parser_smond(a, &parser_state, &parser_mask, parser_byte);
                     break;
                  case M_SIND_NITZ:
                     if (a->count < 8)
                     {
                        parser_sind_nitz(a, &v_sign, parser_byte);
                     }
                     break;
                  case M_COPS:
                     if (parser_state < 7)
                     {
                        if (a->count < 8)
                        {
                           parser_cops(a, &parser_state, parser_byte);
                        }
                        if (parser_state < 7)
                           break;
                     }
                  case M_NO_CARRIER:
                  case M_CUSD:
                  case M_CSQ:
                  case M_CREG:
                  case M_SCID:
                  case M_SIS:
                  case M_SISR:
                  case M_SISW:
                  case M_0_CONNECT_OK:
                  case M_1_CONNECT_OK:
                  case M_0_SEND_OK:
                  case M_1_SEND_OK:
                  case M_RECEIVE:
                  case M_SISI:
                  case M_SICI:
                  case M_SCKS:
                  case M_SCTM:
                  case M_SMGR:
                  case M_SMGL:
                  case M_CMTI_SM:
                  case M_CMTI_ME:
                  case M_CMTI_MT:
                  case M_CGATT:
                  case M_SPIC:
                  case M_FTPGET:
                  case M_JAMMING:
                  case M_P_CMGL:
                  case M_HTTPACTION:
                  case M_HTTPREAD:
                     // разбираем параметры сообщения
                     if (parser_n <= t_msg[parser_row].v_total)
                     {
                        if (parser_byte == ',')
                        {
                           if (a->count < 7)
                              a->count++;
                           v_sign = 0;
                           s_copy = 0;
                           buf[parser_offset++] = 0;
                        }
                        else
                        {
                           parser_mask = (1 << (a->count));

                           if ((t_msg[parser_row].v_type & parser_mask) == 0)
                           {
                              // цифровая переменная для SIM800
                              if ((parser_byte != ',') &&
                                  ((parser_row == M_RECEIVE) || (parser_row == M_FTPGET) || (parser_row == M_HTTPREAD)))
                              {
                                 a->msg[a->count].var = calculate(0, 10, parser_byte, a->msg[a->count].var);
                              }
                              else
                              {
                                 // цифровая переменная
                                 if (parser_byte != '-')
                                 {
                                    a->msg[a->count].var = calculate(v_sign, 10, parser_byte, a->msg[a->count].var);
                                    if (parser_n == 0 && a->count != 0)
                                       parser_n = 1;
                                 }
                                 else
                                 {
                                    // отрицательный знак у переменной
                                    v_sign = -1;
                                 }
                              }
                           }
                           else
                           {
                              // строковая переменная
                              if (buf != 0 && parser_offset < b_size &&
                                  ((s_copy != 0 && parser_byte == ' ') || (parser_byte > 0x20 && parser_byte < 0x7F)))
                              {
                                 if (parser_n == 0)
                                    parser_n = 1;
                                 if (s_copy == 0)
                                 {
                                    if (a->count == 0)
                                    {
                                       parser_offset = 0;
                                       a->msg[0].size = 0;
                                    }

                                    s_copy = 1;
                                    a->msg[a->count].str = buf + parser_offset;
                                 }
                                 buf[parser_offset++] = parser_byte;
                                 a->msg[a->count].size++;
                              }
                           }
                        }
                     }
                     break;
#ifdef USE_BLUETOOTH
                  /* Functions for Bluetooth */
                  case M_BTPAIRING:   //+BTPAIRING: "LG-E612",e8:92:a4:61:bc:65,088094
                     BluetoothInit(buf, wait);
                     break;
                  case M_BTCONNECTING:   //+BTCONNECTING: "e8:92:a4:61:bc:65","SPP"
                     BluetoothConnecting(buf, wait);
                     break;
                  case M_BTCONNECT:
                     BluetoothConnect(buf, wait);   //+BTCONNECT: 1,"LG-E612",e8:92:a4:61:bc:65,"SPP"
                     break;
                  case M_BTDISCONN:
                     BluetoothDisconnect(buf, wait);   //+BTDISCONN: "LG-E612",e8:92:a4:61:bc:65,"SPP"
                     break;

                  case M_BTSPPDATA:   //+BTSPPDATA: 1,6,qwerty
                     for (u8 index_cmd = 0, index_data = 0; index_data < 255; index_data++)
                     {
                        //ишем две запятые
                        parser_ret = gsm_read(&parser_byte, 1, wait);
                        if ((parser_byte == '\r') && (ucSymTrue == 2))
                        {
                           buf[index_cmd] = 0;
                           break;
                        }
                        if ((parser_ret) && (ucSymTrue == 2))
                        {
                           buf[index_cmd] = parser_byte;
                           index_cmd++;
                        }

                        //эта проверка должна быть ниже всех
                        if ((parser_ret) && (ucSymTrue != 2))
                        {
                           if (parser_byte == ',')
                              ucSymTrue++;
                        }
                     }
                     break;
#endif
                  default:
                     start = 0;
                     break;
                  }
               }
               break;
            }
         }
      }

      // строковая переменная, без идентификатора
      if (buf != 0 && p_copy != 0 && parser_offset < b_size && parser_byte > 0x1F && parser_byte < 0x7F)
      {
         buf[parser_offset++] = parser_byte;
      }
   }

   return parser_size;
}
