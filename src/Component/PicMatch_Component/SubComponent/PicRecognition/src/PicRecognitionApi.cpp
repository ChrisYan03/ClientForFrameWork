#include "PicRecognitionApi.h"
#include "RecognitionInstance/PicRecognitionObj.h"

// Global instance of the face recognition manager
static FaceRecognitionManager g_faceRecognitionManager;

PICRECOGNITION_API int PICRECOGNITION_CALL InitFaceRecognition(const char* data_path)
{
    return g_faceRecognitionManager.init(data_path);
}

PICRECOGNITION_API int PICRECOGNITION_CALL DetectFacesInRgba(PicShowInfo* picInfo, FaceDetectionResult* result)
{
    return g_faceRecognitionManager.detectFacesInRgba(picInfo, result);
}

PICRECOGNITION_API void PICRECOGNITION_CALL DestroyFaceRecognition()
{
    g_faceRecognitionManager.destroy();
}