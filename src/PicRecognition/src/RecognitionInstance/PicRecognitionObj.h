#ifndef _FACE_RECOGNITION_MANAGER_H_
#define _FACE_RECOGNITION_MANAGER_H_

#include "PicPlayerDataDef.h"
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>

class FaceRecognitionManager 
{

public:
    FaceRecognitionManager();

    ~FaceRecognitionManager();

    int init(const char* data_path);

    int detectFacesInRgba(PicShowInfo* picInfo, FaceDetectionResult* result);

    void destroy(); 

private:
    cv::CascadeClassifier* face_cascade;
};
#endif // _FACE_RECOGNITION_MANAGER_H_