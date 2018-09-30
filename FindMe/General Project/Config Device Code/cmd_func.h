#ifndef _CMD_FUNC_H_
#define _CMD_FUNC_H_

int CmdUserTel(u8* pansw, u8* parg, u16 len);
int CmdPinSimCard(u8* pansw, u8* parg, u16 len);
int CmdGetNameFirmvare(u8* pansw, u8* parg, u16 len);
int CmdResetDevice(u8* pansw, u8* parg, u16 len);
int CmdResetConfig(u8* pansw, u8* parg, u16 len);
int CmdEraseArchive(u8* pansw, u8* parg, u16 len);
int CmdChangeServAndProt(u8* pansw, u8* parg, u16 len);
int CmdPwrMode(u8* pansw, u8* parg, u16 len);
int CmdSetTimeModeLowPwr1(u8* pansw, u8* parg, u16 len);
int CmdSetTimeModeLowPwr2(u8* pansw, u8* parg, u16 len);
int InfoGsmModemImei(u8* pansw, u8* parg, u16 len);
int InfoSimCardScidFirst(u8* pansw, u8* parg, u16 len);
int InfoSimCardScidSecond(u8* pansw, u8* parg, u16 len);
int ChangeServer1(u8* pansw, u8* parg, u16 len);
int CmdEmrg(u8* pansw, u8* parg, u16 len);
int CmdSensitivityAccel(u8* pansw, u8* parg, u16 len);
int CmdDebugGps(u8* pansw, u8* parg, u16 len);
int CmdDebugGsm(u8* pansw, u8* parg, u16 len);
int ds_init(uint8_t* pansw, uint8_t* parg, uint16_t len);
int ds_print_calib(uint8_t* pansw, uint8_t* parg, uint16_t len);
int CmdFilterCourse(u8* pansw, u8* parg, u16 len);
int CmdFilterDistance(u8* pansw, u8* parg, u16 len);
int CmdFilterMinSpeed(u8* pansw, u8* parg, u16 len);
int CmdFirmLoad(u8* pansw, u8* parg, u16 len);
int CmdTimeFindGps(u8* pansw, u8* parg, u16 len);
#endif