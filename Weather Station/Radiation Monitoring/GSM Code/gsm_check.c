#include "includes.h"
#include "gsm_check.h"
#include "sram.h"

GSM_PARAM gsm_param = { 99, 99, 0, 0, 0, 0, 0, 0 };

RET_INFO GetCellularNetwork(void)
{
   GSM_INFO out_check;

   /* Ускаряет поиск gsm станций и регистрацю в сети (для модема MG23)*/
#ifdef GSM_MODULE_BGS2
   mc("at+cops=2", 10, 1);
   osDelay(1000);
   mc("at+cops=0", 10, 1);
   osDelay(100);
#endif
   /*************************************************/

   // запрос данных о качестве принимаемого сигнала
   if (check_csq(&out_check) == ERR_TIMEOUT)
   {
      return ERR_TIMEOUT;   // GSM cеть не найдена
   }

   // поиск сети
   if (check_creg(&out_check) == ERR_GSMNET)
   {
      return ERR_GSMNET;
   }

   return (RET_INFO)(out_check.msg[1].var + 0x30);
}

void csq_save(GSM_INFO* out)
{
   if (out->m_type == M_CSQ)
   {
      switch (out->count)
      {
      case 2:
         gsm_param.ber = out->msg[1].var;
      case 1:
         gsm_param.rssi = out->msg[0].var;
      }
   }
}

int check_csq(GSM_INFO* out)
{
   uint8_t count = 0;
   if (count == NULL)
   {
      count = DEF_GSM_TIMEOUT;
   }

   for (; count > 0; count--)
   {
      mc_get("at+csq", M_CSQ, out, 1, 1);
      if (out->m_type == M_CSQ)
      {
         // DP_GSM("___CSQ: %d\r", out->msg[0].var);
      }
      if (out->msg[0].var != 99)
      {
         g_stRam.stGsm.aucGsmCsq[0] = out->msg[0].var;
      }
      else
      {
         g_stRam.stGsm.aucGsmCsq[0] = 0;
      }

      if (out->msg[1].var != 99)
      {
         g_stRam.stGsm.aucGsmCsq[1] = out->msg[1].var;
      }
      else
      {
         g_stRam.stGsm.aucGsmCsq[1] = 0;
      }

      if (out->msg[0].var != 99)
      {
         return RET_OK;
      }

      osDelay(1000);
   }
   csq_save(out);

   return ERR_TIMEOUT;
}

// дождаться авторизации в сети получить статус авторизации
// '1' - домашняя сеть, '5' - роуминг.
int check_creg(GSM_INFO* out)
{
   char count = 0;
   if (count == NULL)
   {
      count = DEF_GSM_REG_COUNT;
   }

   for (; count > 0; count--)
   {
      mc_get("at+creg?", M_CREG, out, 1, 1);

      if (out->m_type == M_CREG)
      {
         DP_GSM("__CREG: %d,%d ", out->msg[0].var, out->msg[1].var);
         switch (out->count)
         {
         case 4:
            gsm_param.netCellId = out->msg[3].var;
         case 3:
            gsm_param.netLac = out->msg[2].var;
         case 2:
            gsm_param.regStatus = out->msg[1].var;
         case 1:
            gsm_param.urcMode = out->msg[0].var;
         }
         switch (out->msg[1].var)
         {
         case 0:
            DP_GSM("(error)");
            break;
         case 1:
            DP_GSM("(home)");
            break;
         case 2:
            DP_GSM("(find)");
            break;
         case 3:
            DP_GSM("(ban)");
            break;
         case 5:
            DP_GSM("(rouming)");
            break;
         }
         DP_GSM("\r");
         if (out->msg[1].var == 1 || out->msg[1].var == 5)
         {
            // зарегистрирован
            gsm_param.regStatus = out->msg[1].var;
            return out->msg[1].var;
         }
         if (out->msg[1].var != 2)
         {
            // в регистрации отказано
            break;
         }
         // поиск сети
      }

      osDelay(1000);
   }

   return ERR_GSMNET;
}