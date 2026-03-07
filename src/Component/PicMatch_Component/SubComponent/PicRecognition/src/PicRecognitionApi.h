#ifndef PICRECOGNITIONAPI_H
#define PICRECOGNITIONAPI_H

#include "PicRecognitionDataDef.h"
#include "PicRecognitionGlobal.h"
#include "PicPlayerDataDef.h" // Include the PicShowInfo structure

#ifdef __cplusplus
extern "C" {
#endif

// 初始化人脸识别模块
PICRECOGNITION_API int PICRECOGNITION_CALL InitFaceRecognition(const char* data_path);

// 人脸识别接口 - 输入RGBA数据，返回人脸检测结果
PICRECOGNITION_API int PICRECOGNITION_CALL DetectFacesInRgba(PicShowInfo* picInfo, FaceDetectionResult* result);

// 销毁人脸识别模块
PICRECOGNITION_API void PICRECOGNITION_CALL DestroyFaceRecognition();

#ifdef __cplusplus
}
#endif

#endif // PICRECOGNITIONAPI_H