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

    // 年龄估计函数
    int estimateAge(const cv::Mat& faceImage);

private:
    cv::CascadeClassifier* face_cascade;
    cv::CascadeClassifier* profile_face_cascade;
    cv::dnn::Net faceNet;      // 人脸检测网络
    cv::dnn::Net ageNet;       // 年龄估计网络
    bool use_dnn;
    bool use_age_estimation;   // 是否启用了年龄估计
};
#endif // _FACE_RECOGNITION_MANAGER_H_