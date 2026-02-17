#ifndef _FACE_RECOGNITION_MANAGER_H_
#define _FACE_RECOGNITION_MANAGER_H_

#include "PicPlayerDataDef.h"
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>

class FaceRecognitionManager 
{

public:
    FaceRecognitionManager();

    ~FaceRecognitionManager();

    int init(const char* data_path);

    int detectFacesInRgba(PicShowInfo* picInfo, FaceDetectionResult* result);

    void destroy(); 

private:
    // 人脸质量评估函数
    float calculateFaceQuality(const cv::Mat& grayImage, const cv::Rect& faceRect, bool isProfile = false);

    // 使用DNN进行人脸检测
    std::vector<cv::Rect> detectFacesWithDNN(const cv::Mat& image);

private:
    cv::CascadeClassifier* face_cascade;
    cv::CascadeClassifier* profile_face_cascade;
    cv::dnn::Net net;
    bool use_dnn;
};
#endif // _FACE_RECOGNITION_MANAGER_H_