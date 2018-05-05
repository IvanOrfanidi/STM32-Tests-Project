
#ifndef __IMG_H
#define __IMG_H

#ifdef __cplusplus
extern "C" {
#endif

#define SIZE_ARR_IMG_WEL 6

typedef __packed struct
{
   uint8_t ImgHeight; /*!< Image height in pixels */
   uint8_t ImgWidth; /*!< Image width in pixels */

   const char* pData; /*!< Pointer to data font data array */
} ImgDef_t;

extern const char* ptrArcheryWel[];
extern ImgDef_t ImgBatFull;
extern ImgDef_t ImgBatHigh;
extern ImgDef_t ImgBatMedium;
extern ImgDef_t ImgBatLow;
extern ImgDef_t ImgBatNone;
extern ImgDef_t ImgBatCharge;
extern ImgDef_t ImgBatFail;
extern ImgDef_t ImgSoundOn;
extern ImgDef_t ImgSoundOff;
extern ImgDef_t ImgShutdown;

#ifdef __cplusplus
}
#endif

#endif /* __IMG_H */