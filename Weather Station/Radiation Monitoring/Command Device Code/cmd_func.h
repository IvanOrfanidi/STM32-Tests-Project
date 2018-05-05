#ifndef _CMD_FUNC_H_
#define _CMD_FUNC_H_

int CmdGetNameFirmvare(u8* pansw, u8* parg, u16 len);
int CmdResetDevice(u8* pansw, u8* parg, u16 len);
int CmdResetConfig(u8* pansw, u8* parg, u16 len);
int CmdEraseArchive(u8* pansw, u8* parg, u16 len);
int CmdPwrMode(u8* pansw, u8* parg, u16 len);
int CmdSetTimeModeLowPwr1(u8* pansw, u8* parg, u16 len);
int CmdSetTimeModeLowPwr2(u8* pansw, u8* parg, u16 len);
int CmdChangePinSimCard(u8* pansw, u8* parg, u16 len);
int InfoGsmModemImei(u8* pansw, u8* parg, u16 len);
int InfoSimCardScid(u8* pansw, u8* parg, u16 len);
int CmdRoamingEnable(u8* pansw, u8* parg, u16 len);
int ChangeServer1(u8* pansw, u8* parg, u16 len);
int CmdSensitivityAccel(u8* pansw, u8* parg, u16 len);
int CmdDebugGps(u8* pansw, u8* parg, u16 len);
int CmdDebugGsm(u8* pansw, u8* parg, u16 len);
int ds_init(uint8_t* pansw, uint8_t* parg, uint16_t len);
int ds_print_calib(uint8_t* pansw, uint8_t* parg, uint16_t len);
#endif