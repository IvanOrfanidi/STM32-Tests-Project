/**
 ******************************************************************************
 * @file    net.cpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    07/01/2019
 * @brief   This file provides all the SPI firmware method.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; </center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "net.hpp"


void Net::MakeEth(uint8_t* buf, const uint8_t* macaddr)
{
    uint8_t i = 0;

    //copy the destination mac from the source and fill my mac into src
    while(i < Enc::MAC_ADDR_SIZE)
    {
        buf[ETH_DST_MAC + i] = buf[ETH_SRC_MAC + i];
        buf[ETH_SRC_MAC + i] = macaddr[i];
        i++;
    }
}


void Net::MakeIp(uint8_t* buf, const uint8_t* ipaddr)
{
    uint8_t i=0;

    while(i < Enc::IP_ADDR_SIZE)
    {
        buf[IP_DST_P + i] = buf[IP_SRC_P + i];
        buf[IP_SRC_P + i] = ipaddr[i];
        i++;
    }
    FillIpHdrChecksum(buf);
}


void Net::FillIpHdrChecksum(uint8_t* buf)
{
    // clear the 2 byte checksum
    buf[IP_CHECKSUM_P] = 0;
    buf[IP_CHECKSUM_P+1] = 0;
    buf[IP_FLAGS_P] = 0x40; // don't fragment
    buf[IP_FLAGS_P+1] = 0;  // fragement offset
    buf[IP_TTL_P] = 64; // ttl
    
    // calculate the checksum:
    uint16_t crc = CalcCrc(&buf[IP_P], IP_HEADER_LEN, PacketType_t::IP);
    buf[IP_CHECKSUM_P] = crc >> 8;
    buf[IP_CHECKSUM_P+1] = crc & 0xff;
}


uint16_t Net::CalcCrc(uint8_t* buf, size_t len, PacketType_t type)
{
    uint32_t sum = 0;
    if(type == PacketType_t::UDP)
    {
        sum += IP_PROTO_UDP_V; // protocol udp
        // the length here is the length of udp (data+header len)
        // =length given to this function - (IP.scr+IP.dst length)
        sum += (len - 8); // = real tcp len
    }
    if(type == PacketType_t::TCP)
    {
        sum += IP_PROTO_TCP_V; 
        // the length here is the length of tcp (data+header len)
        // =length given to this function - (IP.scr+IP.dst length)
        sum += (len - 8); // = real tcp len
    }
    // build the sum of 16bit words
    while(len > 1)
    {
        sum += 0xFFFF & ((*buf << 8) | *(buf+1));
        buf += 2;
        len -= 2;
    }
    // if there is a byte left then add it (padded with zero)
    if (len > 0)
    {
        sum += ((0xFF & *buf) << 8);
    }
    // now calculate the sum over the bytes in the sum
    // until the result is only 16bit long
    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    // build 1's complement:
    return ((uint16_t)sum ^ 0xFFFF);
}


bool Net::EthTypeIsIcmpEcho(uint8_t* buf, size_t len)
{
    if(0 == len) {
        return false;
    }
    
    return ((buf[IP_PROTO_P] == IP_PROTO_ICMP_V) && (buf[ICMP_TYPE_P] == ICMP_TYPE_ECHOREQUEST_V));
}


bool Net::EthTypeIsIp(uint8_t* buf, size_t len, const uint8_t* ipaddr)
{
    //eth+ip+udp header is 42
    if(len < 42) {
        return false;
    }
    
    if((buf[ETH_TYPE_H_P] != ETHTYPE_IP_H_V) || 
       (buf[ETH_TYPE_L_P]!=ETHTYPE_IP_L_V)) {
        return false;
    }
    
    if(buf[IP_HEADER_LEN_VER_P] != 0x45) {
        // must be IP V4 and 20 byte header
        return false;
    }

    if(memcmp(&buf[IP_DST_P], ipaddr, Enc::IP_ADDR_SIZE) == 0) {
        return true;
    }
    return false;
}


bool Net::EthTypeIsArp(uint8_t* buf, size_t len, const uint8_t* ipaddr)
{
    if(len < 41) {
        return false;
    }
    
    if((buf[ETH_TYPE_H_P] != ETHTYPE_ARP_H_V) || 
       (buf[ETH_TYPE_L_P] != ETHTYPE_ARP_L_V)) {
        return false;
    }
    
    if(memcmp(&buf[ETH_ARP_DST_IP_P], ipaddr, Enc::IP_ADDR_SIZE) == 0) {
        return true;
    }
    return false;
}

        
size_t Net::MakeArpAnswerFromRequest(uint8_t* buf, size_t len, const uint8_t* macaddr, const uint8_t* ipaddr)
{
   size_t i = 0;

   MakeEth(buf, macaddr);
   
   buf[ETH_ARP_OPCODE_H_P] = ETH_ARP_OPCODE_REPLY_H_V;
   buf[ETH_ARP_OPCODE_L_P] = ETH_ARP_OPCODE_REPLY_L_V;
   // fill the mac addresses:
   while(i < Enc::MAC_ADDR_SIZE)
   {
      buf[ETH_ARP_DST_MAC_P + i] = buf[ETH_ARP_SRC_MAC_P + i];
      buf[ETH_ARP_SRC_MAC_P + i] = macaddr[i];
      i++;
   }
   
   i = 0;
   while(i < Enc::IP_ADDR_SIZE)
   {
      buf[ETH_ARP_DST_IP_P + i] = buf[ETH_ARP_SRC_IP_P + i];
      buf[ETH_ARP_SRC_IP_P + i] = ipaddr[i];
      i++;
   }
   // eth+arp is 42 bytes:
   return 42;
}


size_t Net::MakeIcmpEchoAnswerFromRequest(uint8_t* buf, size_t len, const uint8_t* macaddr, const uint8_t* ipaddr)
{
    MakeEth(buf, macaddr);
    MakeIp(buf, ipaddr);
    
    buf[ICMP_TYPE_P] = ICMP_TYPE_ECHOREPLY_V;
    // we changed only the icmp.type field from request(=8) to reply(=0).
    // we can therefore easily correct the checksum:
    if (buf[ICMP_CHECKSUM_P] > (0xff - 0x08))
    {
        buf[ICMP_CHECKSUM_P + 1]++;
    }
    buf[ICMP_CHECKSUM_P] += 0x08;
    return len;
}
