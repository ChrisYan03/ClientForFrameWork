#include "PicRecognitionObj.h"
#include <opencv2/imgproc.hpp>
#include <vector>
#include <algorithm>
#include "LogUtil.h"

#define MAX_FACE_COUNT 3

struct FaceQuality {
    cv::Rect rect;
    float qualityScore;
    float confidence;
    bool isProfile;
    int detectionType;
    
    FaceQuality(const cv::Rect& r, float score, float conf, bool profile = false, int type = 0)
        : rect(r), qualityScore(score), confidence(conf), isProfile(profile), detectionType(type) {}
    
    bool operator<(const FaceQuality& other) const {
        return qualityScore > other.qualityScore;
    }
};

FaceRecognitionManager::FaceRecognitionManager()
    : face_cascade(nullptr)
    , profile_face_cascade(nullptr)
{
}

FaceRecognitionManager::~FaceRecognitionManager() 
{
    destroy();
}

int FaceRecognitionManager::init(const char* data_path) 
{
    try {
        LogUtil::initLogger("PicRecognition");
        face_cascade = new cv::CascadeClassifier();
        profile_face_cascade = new cv::CascadeClassifier();
        
        std::string exe_dir(data_path);
        std::vector<std::string> frontalPaths = {
            exe_dir + "/data/haarcascades/haarcascade_frontalface_default.xml",
            exe_dir + "/haarcascade_frontalface_default.xml"
        };

        std::vector<std::string> profilePaths = {
            exe_dir + "/data/haarcascades/haarcascade_profileface.xml",
            exe_dir + "/haarcascade_profileface.xml"
        };

        bool frontalLoaded = false;
        for (const auto& path : frontalPaths) {
            if (face_cascade->load(path)) {
                LOG_INFO("Loaded frontal face detector: {}", path);
                frontalLoaded = true;
                break;
            }
        }

        if (!frontalLoaded) {
            LOG_ERROR("Failed to load frontal face detector");
            delete face_cascade;
            face_cascade = nullptr;
            delete profile_face_cascade;
            profile_face_cascade = nullptr;
            return -1;
        }

        bool profileLoaded = false;
        for (const auto& path : profilePaths) {
            if (profile_face_cascade->load(path)) {
                LOG_INFO("Loaded profile face detector: {}", path);
                profileLoaded = true;
                break;
            }
        }

        if (!profileLoaded) {
            LOG_WARN("Profile face detector not available, using frontal detection only");
        }

        LOG_INFO("Face recognition initialized successfully");
        return 0;
    }
    catch (...) {
        if (face_cascade) {
            delete face_cascade;
            face_cascade = nullptr;
        }
        if (profile_face_cascade) {
            delete profile_face_cascade;
            profile_face_cascade = nullptr;
        }
        return -1;
    }
}

// 简单快速的质量评估
float FaceRecognitionManager::calculateFaceQuality(const cv::Mat& grayImage, const cv::Rect& faceRect, bool isProfile) {
    try {
        cv::Rect validRect = faceRect & cv::Rect(0, 0, grayImage.cols, grayImage.rows);
        if (validRect.area() <= 0) return 0.0f;
        
        cv::Mat faceRegion = grayImage(validRect);
        
        // 快速质量评估：主要看大小和位置
        float sizeRatio = static_cast<float>(faceRect.width * faceRect.height) / 
                         static_cast<float>(grayImage.cols * grayImage.rows);
        float sizeScore = 1.0f - std::abs(sizeRatio - 0.05f) * 5.0f;
        sizeScore = std::max(0.0f, std::min(1.0f, sizeScore));
        
        float centerX = static_cast<float>(faceRect.x + faceRect.width/2) / grayImage.cols;
        float centerY = static_cast<float>(faceRect.y + faceRect.height/2) / grayImage.rows;
        float centerScore = 1.0f - std::sqrt(std::pow(centerX - 0.5f, 2) + std::pow(centerY - 0.5f, 2));
        
        float profilePenalty = isProfile ? 0.9f : 1.0f;
        
        return std::max(0.0f, std::min(1.0f, (sizeScore * 0.7f + centerScore * 0.3f) * profilePenalty));
    }
    catch (...) {
        return 0.0f;
    }
}

int FaceRecognitionManager::detectFacesInRgba(PicShowInfo* picInfo, FaceDetectionResult* result)
{
    if (!picInfo || !result || !face_cascade) {
        return -1;
    }

    try {
        if (result->faces) {
            delete[] result->faces;
            result->faces = nullptr;
        }
        result->faceCount = 0;

        if (!picInfo->imageRgbaData || picInfo->picWidth == 0 || picInfo->picHeight == 0) {
            return -1;
        }

        cv::Mat rgbaMat(picInfo->picHeight, picInfo->picWidth, CV_8UC4, 
                        reinterpret_cast<unsigned char*>(picInfo->imageRgbaData));
        cv::Mat bgrMat, grayMat;
        cv::cvtColor(rgbaMat, bgrMat, cv::COLOR_RGBA2BGR);
        cv::cvtColor(bgrMat, grayMat, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(grayMat, grayMat);

        std::vector<FaceQuality> allFaces;

        // 正面人脸检测
        std::vector<cv::Rect> frontalFaces;
        face_cascade->detectMultiScale(grayMat, frontalFaces, 1.1, 3, 0, cv::Size(30, 30));
        
        for (const auto& face : frontalFaces) {
            float quality = calculateFaceQuality(grayMat, face, false);
            float confidence = 0.8f + 0.2f * quality;
            allFaces.emplace_back(face, quality, confidence, false, 0);
        }

        // 侧脸检测 - 简化版本
        if (profile_face_cascade) {
            // 左侧脸
            std::vector<cv::Rect> leftFaces;
            profile_face_cascade->detectMultiScale(grayMat, leftFaces, 1.1, 2, 0, cv::Size(25, 25));
            for (const auto& face : leftFaces) {
                float quality = calculateFaceQuality(grayMat, face, true) * 0.85f;
                float confidence = 0.6f + 0.2f * quality;
                allFaces.emplace_back(face, quality, confidence, true, 1);
            }
            
            // 右侧脸（图像翻转）
            cv::Mat flipped;
            cv::flip(grayMat, flipped, 1);
            std::vector<cv::Rect> rightFaces;
            profile_face_cascade->detectMultiScale(flipped, rightFaces, 1.1, 2, 0, cv::Size(25, 25));
            for (auto& face : rightFaces) {
                face.x = grayMat.cols - face.x - face.width;
                float quality = calculateFaceQuality(grayMat, face, true) * 0.85f;
                float confidence = 0.6f + 0.2f * quality;
                allFaces.emplace_back(face, quality, confidence, true, 2);
            }
        }

        // 简单去重
        std::sort(allFaces.begin(), allFaces.end());
        std::vector<FaceQuality> uniqueFaces;
        
        for (const auto& face : allFaces) {
            bool overlap = false;
            for (const auto& existing : uniqueFaces) {
                cv::Rect intersection = face.rect & existing.rect;
                cv::Rect uni = face.rect | existing.rect;
                if (uni.area() > 0 && static_cast<float>(intersection.area()) / uni.area() > 0.3f) {
                    overlap = true;
                    break;
                }
            }
            if (!overlap) {
                uniqueFaces.push_back(face);
            }
        }

        size_t finalCount = std::min(static_cast<size_t>(MAX_FACE_COUNT), uniqueFaces.size());
        strncpy(result->imageId, picInfo->imageId, IMAGE_ID_LEN - 1);
        result->imageId[IMAGE_ID_LEN - 1] = '\0';
        result->faceCount = static_cast<int>(finalCount);

        if (finalCount > 0) {
            result->faces = new FaceInfo[finalCount];
            for (size_t i = 0; i < finalCount; ++i) {
                const FaceQuality& fq = uniqueFaces[i];
                result->faces[i].x = static_cast<float>(fq.rect.x) / picInfo->picWidth;
                result->faces[i].y = static_cast<float>(fq.rect.y) / picInfo->picHeight;
                result->faces[i].width = static_cast<float>(fq.rect.width) / picInfo->picWidth;
                result->faces[i].height = static_cast<float>(fq.rect.height) / picInfo->picHeight;
                result->faces[i].confidence = fq.confidence;
            }
        }

        return 0;
    }
    catch (...) {
        if (result->faces) {
            delete[] result->faces;
            result->faces = nullptr;
        }
        result->faceCount = 0;
        return -1;
    }
}

void FaceRecognitionManager::destroy() 
{
    if (face_cascade) {
        delete face_cascade;
        face_cascade = nullptr;
    }
    if (profile_face_cascade) {
        delete profile_face_cascade;
        profile_face_cascade = nullptr;
    }
}