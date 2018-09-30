#ifndef __FINDME_911_H
#define __FINDME_911_H

#ifdef __cplusplus
extern "C" {
#endif

#define MIN_SIZE_NUMBER_SMS 5   //��������� �������� ������ ������ 5 ����.
#define MAX_SIZE_NAME_DEVICE 15   //������������ ��� �������� ������� �� �������.

#define MAX_SIZE_LBS_FOR_FM911 6
#define SIZE_LBS_FOR_FM911 6   //�� ����� 6
#if (SIZE_LBS_FOR_FM911 > MAX_SIZE_LBS_FOR_FM911)
#   error "The size LBS of the stations to the server FM911 is exceeded!"
#endif

#define _OLD_POSITION_GPS_ FALSE   // �������� ������� ��������� ��� ��������� ������

#define TIME_STANDBY 31556926

#define TIME_WAIT_SMS_REG_USER (15 * 60)   // wait sms registration (min)

#define DEFAULT_PINCODE_SIM "0000"

typedef enum
{
   TYPE_REG_USER = 'R',   // ����������� ������������ �� �������
   TYPE_REG_TO_BASE = 'B',   // ����������� ���������� � ���� ������ ������� � �������� �����������
   // TYPE_ERR_POWER =             'D',     // GSM ������ ���������� ��-�� ����������� ���������� �������
   TYPE_WU_START = 'A',   // ���������� ������� �� ������� ���������� �������(�������� ������� ��� � ������)
   TYPE_WU_BUTTON = 'E',   // ������������ �� ������
   TYPE_WU_TIMER = 'T',   // ����������� �� �������
   TYPE_ERR_COLD = 'H',   // ������ �����������
   TYPE_ERR_CONSRV = 'X',   // �� ������� ����� �� �������, ��������� ����� ������
   TYPE_MOVE_START = 'M',   // ������ ��������
   TYPE_MOVE_STOP = 'N',   // ����������
   TYPE_GOODBYE = 'J'   // ���� � ������� 911 �� ����� � ������ ���������
} T_TYPE_CONNECT;

typedef enum
{
   ANS_OK = 0,
   SRV_TIMEOUT = 1,
   ANS_ERROR = 2,
   CRC_ERROR = 3
} T_ANS_SRV_FM911;

int FrameBuildGpsDataFm911(char* pOut, int OffsetData);
int FrameBuildSystemFm911(char* pOut, int OffsetData);
int FrameBuildGsmDataFm911(char* pOut, int OffsetData);
int FrameBuildAccelDataFm911(char* pOut, int OffsetData);
int FrameBuildDeviceDataFm911(char* pOut, int OffsetData);
int FrameBuildLogDataFm911(char* pOut, int OffsetData);
int FrameBuildAccelDataFm911(char* pOut, int OffsetData);

T_ANS_SRV_FM911 GprsAckDataFm911(void);
int CheckSmsRegUserFm911(char* ptrNumUserTel, char* ptrNameDevice, _Bool* pCode);

T_TYPE_CONNECT GetTypeConnectFm911(void);
uint8_t gsm_cs(char* buf, int size);
int getNameFmVersia(char* ptr);

#ifdef __cplusplus
}
#endif

#endif