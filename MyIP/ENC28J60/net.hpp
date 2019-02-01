/**
 ******************************************************************************
 * @file    net.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    07/01/2019
 * @brief   This file provides all the ENC28J60 firmware method.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; </center></h2>
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NET_HPP
#define __NET_HPP

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "enc28j60.hpp"

namespace Net {
    // notation: _P = position of a field
    //           _V = value of a field
    enum {
        // ******* ETH *******
        ETH_HEADER_LEN = 14,
        // values of certain bytes:
        ETHTYPE_ARP_H_V = 0x08,
        ETHTYPE_ARP_L_V = 0x06,
        ETHTYPE_IP_V = 0x0800,
        ETHTYPE_IP_H_V = 0x08,
        ETHTYPE_IP_L_V = 0x00,
        // byte positions in the ethernet frame:
        //
        // Ethernet type field (2bytes):
        ETH_TYPE_H_P = 12,
        ETH_TYPE_L_P = 13,
        //
        ETH_DST_MAC = 0,
        ETH_SRC_MAC = 6,

        // ******* ARP *******
        ETH_ARP_OPCODE_REPLY_H_V = 0x0,
        ETH_ARP_OPCODE_REPLY_L_V = 0x02,

        // arp.dst.ip
        ETH_ARP_DST_IP_P = 0x26,
        // arp.opcode
        ETH_ARP_OPCODE_H_P = 0x14,
        ETH_ARP_OPCODE_L_P = 0x15,
        // arp.src.mac
        ETH_ARP_SRC_MAC_P = 0x16,
        ETH_ARP_SRC_IP_P = 0x1c,
        ETH_ARP_DST_MAC_P = 0x20,

        ARP_OPCODE_REQUEST_H_V = 0x00,
        ARP_OPCODE_REQUEST_L_V = 0x01,
        ARP_OPCODE_REPLY_H_V = 0x00,
        ARP_OPCODE_REPLY_L_V = 0x02,

        ARP_HARDWARE_TYPE_H_V = 0x00,
        ARP_HARDWARE_TYPE_L_V = 0x01,
        ARP_PROTOCOL_H_V = 0x08,
        ARP_PROTOCOL_L_V = 0x00,
        ARP_HARDWARE_SIZE_V = 0x06,
        ARP_PROTOCOL_SIZE_V = 0x04,

        ARP_HARDWARE_TYPE_H_P = 0x0E,
        ARP_HARDWARE_TYPE_L_P = 0x0F,
        ARP_PROTOCOL_H_P = 0x10,
        ARP_PROTOCOL_L_P = 0x11,
        ARP_HARDWARE_SIZE_P = 0x12,
        ARP_PROTOCOL_SIZE_P = 0x13,
        ARP_OPCODE_H_P = 0x14,
        ARP_OPCODE_L_P = 0x15,
        ARP_SRC_MAC_P = 0x16,
        ARP_SRC_IP_P = 0x1C,
        ARP_DST_MAC_P = 0x20,
        ARP_DST_IP_P = 0x26,

        // ******* IP *******
        IP_HEADER_LEN = 20,

        IP_PROTO_ICMP_V = 0x01,
        IP_PROTO_TCP_V = 0x06,
        IP_PROTO_UDP_V = 0x11,
        IP_V4_V = 0x40,
        IP_HEADER_LENGTH_V = 0x05,

        IP_P = 0x0E,
        IP_HEADER_VER_LEN_P = 0x0E,
        IP_TOS_P = 0x0F,
        IP_TOTLEN_H_P = 0x10,
        IP_TOTLEN_L_P = 0x11,
        IP_ID_H_P = 0x12,
        IP_ID_L_P = 0x13,
        IP_FLAGS_P = 0x14,
        IP_FLAGS_H_P = 0x14,
        IP_FLAGS_L_P = 0x15,
        IP_TTL_P = 0x16,
        IP_PROTO_P = 0x17,
        IP_CHECKSUM_P = 0x18,
        IP_CHECKSUM_H_P = 0x18,
        IP_CHECKSUM_L_P = 0x19,
        IP_SRC_IP_P = 0x1A,
        IP_DST_IP_P = 0x1E,

        IP_SRC_P = 0x1A,
        IP_DST_P = 0x1E,
        IP_HEADER_LEN_VER_P = 0x0E,
        // ******* ICMP *******
        ICMP_TYPE_ECHOREPLY_V = 0,
        ICMP_TYPE_ECHOREQUEST_V = 8,
        //
        ICMP_TYPE_P = 0x22,
        ICMP_CHECKSUM_P = 0x24,

        // ******* UDP *******
        UDP_HEADER_LEN = 8,
        //
        UDP_SRC_PORT_H_P = 0x22,
        UDP_SRC_PORT_L_P = 0x23,
        UDP_DST_PORT_H_P = 0x24,
        UDP_DST_PORT_L_P = 0x25,
        //
        UDP_LEN_H_P = 0x26,
        UDP_LEN_L_P = 0x27,
        UDP_CHECKSUM_H_P = 0x28,
        UDP_CHECKSUM_L_P = 0x29,
        UDP_DATA_P = 0x2A,

        // ******* TCP *******
        //  plain len without the options:
        TCP_HEADER_LEN_PLAIN = 20,

        TCP_FLAG_FIN_V = 0x01,
        TCP_FLAGS_FIN_V = 0x01,
        TCP_FLAGS_SYN_V = 0x02,
        TCP_FLAG_SYN_V = 0x02,
        TCP_FLAG_RST_V = 0x04,
        TCP_FLAG_PUSH_V = 0x08,
        TCP_FLAGS_ACK_V = 0x10,
        TCP_FLAG_ACK_V = 0x10,
        TCP_FLAG_URG_V = 0x20,
        TCP_FLAG_ECE_V = 0x40,
        TCP_FLAG_CWR_V = 0x80,
        TCP_FLAGS_SYNACK_V = 0x12,

        TCP_SRC_PORT_H_P = 0x22,
        TCP_SRC_PORT_L_P = 0x23,
        TCP_DST_PORT_H_P = 0x24,
        TCP_DST_PORT_L_P = 0x25,
        TCP_SEQ_P = 0x26,    // the tcp seq number is 4 bytes 0x26-0x29
        TCP_SEQ_H_P = 0x26,
        TCP_SEQACK_P = 0x2A,    // 4 bytes
        TCP_SEQACK_H_P = 0x2A,
        TCP_HEADER_LEN_P = 0x2E,
        TCP_FLAGS_P = 0x2F,
        TCP_FLAG_P = 0x2F,
        TCP_WINDOWSIZE_H_P = 0x30,    // 2 bytes
        TCP_WINDOWSIZE_L_P = 0x31,
        TCP_CHECKSUM_H_P = 0x32,
        TCP_CHECKSUM_L_P = 0x33,
        TCP_URGENT_PTR_H_P = 0x34,    // 2 bytes
        TCP_URGENT_PTR_L_P = 0x35,
        TCP_OPTIONS_P = 0x36,
        TCP_DATA_P = 0x36
    };

    enum class PacketType_t {
        IP,
        UDP,
        TCP
    };

    enum Size_t {
        ETH_HEADER_SIZE = 42
    };

    bool EthTypeIsArp(uint8_t*, size_t, const uint8_t*);

    bool EthTypeIsIp(uint8_t*, size_t, const uint8_t*);

    bool EthTypeIsIcmpEcho(uint8_t*, size_t);

    void MakeEth(uint8_t*, const uint8_t*);

    void MakeIp(uint8_t*, const uint8_t*);

    uint16_t CalcCrc(uint8_t*, size_t, PacketType_t);

    void FillIpHdrChecksum(uint8_t*);

    size_t MakeArpAnswerFromRequest(uint8_t*, size_t, const uint8_t*, const uint8_t*);

    size_t MakeIcmpEchoAnswerFromRequest(uint8_t*, size_t, const uint8_t*, const uint8_t*);
};    // namespace Net

#endif