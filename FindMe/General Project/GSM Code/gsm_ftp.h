#ifndef _GSM_FTP_H
#define _GSM_FTP_H

#include "includes.h"
#include "ee_variable.h"

#define TIMEOUT_FTP_CONNECT 30    //Ждем подключения

RET_INFO DownloadFirmware(void);
RET_INFO ProfileFtpSocketInit(void);
RET_INFO profile_ftp(u8 nProf);
int ftp_or_http_read(uint8_t nProf, char* buf, int size);
FRAME_FIRMWARE_TYPE ftp_socket_read_firmware(void);
#endif