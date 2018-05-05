

#include "ethernet_general.h"
#include "includes.h"

#define BUF ((struct uip_eth_hdr*)&uip_buf[0])

void vTask_uIP_periodic(void* pvParameters)
{
   uint32_t i;
   uint8_t delay_arp = 0;

   struct uip_eth_addr mac = { { 0x00, 0x21, 0x68, 0x01, 0x00, 0x29 } };

   enc28j60_init(mac.addr);

   // инициализация стека
   uip_init();
   uip_arp_init();

   // инициализация приложения, потом втулим сюда веб-сервер
   hello_world_init();

   // установим наш МАС
   uip_setethaddr(mac);

   // установим адрес хоста (не используем dhcp)
   uip_ipaddr_t ipaddr;
   uip_ipaddr(ipaddr, 192, 168, 105, 100);
   uip_sethostaddr(ipaddr);
   uip_ipaddr(ipaddr, 192, 168, 1, 5);
   uip_setdraddr(ipaddr);
   uip_ipaddr(ipaddr, 255, 255, 255, 0);
   uip_setnetmask(ipaddr);

   osDelay(2000);

   while (1)
   {
      vTaskDelay(configTICK_RATE_HZ / 2);   // полсекунды
      delay_arp++;
      for (i = 0; i < UIP_CONNS; i++)
      {
         uip_periodic(i);
         if (uip_len > 0)
         {
            uip_arp_out();
            enc28j60_send_packet((uint8_t*)uip_buf, uip_len);
         }
      }

#if UIP_UDP
      for (i = 0; i < UIP_UDP_CONNS; i++)
      {
         uip_udp_periodic(i);
         if (uip_len > 0)
         {
            uip_arp_out();
            // network_send();
         }
      }
#endif   // UIP_UDP

      if (delay_arp >= 50)
      {   // один раз за 50 проходов цикла, около 10 сек.
         delay_arp = 0;
         uip_arp_timer();
      }
   }
}
//--------------------------------------------------------------
void vTask_uIP(void* pvParameters)
{
   while (1)
   {
      uip_len = enc28j60_recv_packet((uint8_t*)uip_buf, UIP_BUFSIZE);

      if (uip_len > 0)
      {
         if (BUF->type == htons(UIP_ETHTYPE_IP))
         {
            uip_arp_ipin();
            uip_input();
            if (uip_len > 0)
            {
               uip_arp_out();
               enc28j60_send_packet((uint8_t*)uip_buf, uip_len);
            }
         }
         else if (BUF->type == htons(UIP_ETHTYPE_ARP))
         {
            uip_arp_arpin();
            if (uip_len > 0)
            {
               enc28j60_send_packet((uint8_t*)uip_buf, uip_len);
            }
         }
      }
      taskYIELD();
   }
}

void uip_log(char* msg)
{
}