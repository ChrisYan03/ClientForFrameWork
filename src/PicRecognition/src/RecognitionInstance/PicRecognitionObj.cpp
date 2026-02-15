#include "PicRecognitionObj.h"
#include <opencv2/imgproc.hpp>
#include <vector>
#include "LogUtil.h"
FaceRecognitionManager::FaceRecognitionManager()
    : face_cascade(nullptr)
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
        // 创建分类器对象
        face_cascade = new cv::CascadeClassifier();
        
        // 尝试加载人脸检测模型
        // 在不同可能的位置查找 haarcascade_frontalface_default.xml 文件
        std::string exe_dir(data_path);
        std::vector<std::string> cascadePaths = {
            exe_dir + "/data/haarcascades/haarcascade_frontalface_default.xml"
        };

        bool modelLoaded = false;
        for (const auto& path : cascadePaths) {
            if (face_cascade->load(path)) {
                LOG_INFO("Successfully loaded cascade classifier from: {}", path);
                modelLoaded = true;
                break;
            }
        }

        if (!modelLoaded) {
            LOG_ERROR("Could not load cascade classifier from any path!");
            std::string expectedPaths;
            for (const auto& path : cascadePaths) {
                expectedPaths += path + "; ";
            }
            LOG_ERROR("Expected paths: {}", expectedPaths);
            
            delete face_cascade;
            face_cascade = nullptr;
            return -1;
        }

        LOG_INFO("Face recognition module initialized successfully");
        return 0; // 成功初始化
    }
    catch (const cv::Exception& e) {
        LOG_ERROR("Error initializing face recognition: {}", e.what());
        if (face_cascade) {
            delete face_cascade;
            face_cascade = nullptr;
        }
        return -1;
    }
    catch (const std::exception& e) {
        LOG_ERROR("General error initializing face recognition: {}", e.what());
        if (face_cascade) {
            delete face_cascade;
            face_cascade = nullptr;
        }
        return -1;
    }
    catch (...) {
        LOG_ERROR("Unknown error occurred during face recognition initialization");
        if (face_cascade) {
            delete face_cascade;
            face_cascade = nullptr;
        }
        return -1;
    }
}

int FaceRecognitionManager::detectFacesInRgba(PicShowInfo* picInfo, FaceDetectionResult* result)
 {
    if (!picInfo || !result || !face_cascade) {
        LOG_ERROR("Invalid input parameters: picInfo={}, result={}, face_cascade={}", 
                    picInfo ? "valid" : "null", 
                    result ? "valid" : "null", 
                    face_cascade ? "valid" : "null");
        return -1;
    }

    try {
        // Free existing memory if any
        if (result->faces) {
            delete[] result->faces;
            result->faces = nullptr;
        }
        result->faceCount = 0;

        // Check if input data is valid
        if (!picInfo->imageRgbaData || picInfo->picWidth == 0 || picInfo->picHeight == 0) {
            LOG_ERROR("Invalid image data: imageRgbaData={}, picWidth={}, picHeight={}", 
                        picInfo->imageRgbaData ? "valid" : "null", 
                        picInfo->picWidth, 
                        picInfo->picHeight);
            return -1;
        }

        // Create OpenCV Mat object using RGBA data
        cv::Mat rgbaMat(picInfo->picHeight, picInfo->picWidth, CV_8UC4, 
                        reinterpret_cast<unsigned char*>(picInfo->imageRgbaData));

        // Convert RGBA to BGR format because OpenCV expects BGR
        cv::Mat bgrMat;
        cv::cvtColor(rgbaMat, bgrMat, cv::COLOR_RGBA2BGR);

        // Convert to grayscale for face detection
        cv::Mat grayMat;
        cv::cvtColor(bgrMat, grayMat, cv::COLOR_BGR2GRAY);

        // Histogram equalization to improve contrast
        cv::equalizeHist(grayMat, grayMat);

        // Store detected face rectangles
        std::vector<cv::Rect> faces;
        
        // Perform face detection
        // Parameters:
        // grayMat: input grayscale image
        // faces: output vector of face rectangles
        // 1.1: scale factor for image pyramid
        // 3: minimum neighbors threshold
        // 0: flags (used in older OpenCV versions)
        // cv::Size(30, 30): minimum detection window size
        face_cascade->detectMultiScale(grayMat, faces, 1.1, 3, 0, cv::Size(30, 30));

        // Set the image ID from the input picInfo
        strncpy(result->imageId, picInfo->imageId, IMAGE_ID_LEN - 1);
        result->imageId[IMAGE_ID_LEN - 1] = '\0';

        // Get the number of detected faces
        int detectedCount = static_cast<int>(faces.size());
        result->faceCount = detectedCount;

        LOG_DEBUG("Face detection completed: {} faces detected for image ID: {}", 
                    detectedCount, picInfo->imageId);

        // Allocate memory for face info array if faces were detected
        if (detectedCount > 0) {
            result->faces = new FaceInfo[detectedCount];

            // Convert detection results to normalized coordinates
            for (int i = 0; i < detectedCount; ++i) {
                cv::Rect& face = faces[i];
                
                // Calculate normalized coordinates
                result->faces[i].x = static_cast<float>(face.x) / static_cast<float>(picInfo->picWidth);
                result->faces[i].y = static_cast<float>(face.y) / static_cast<float>(picInfo->picHeight);
                result->faces[i].width = static_cast<float>(face.width) / static_cast<float>(picInfo->picWidth);
                result->faces[i].height = static_cast<float>(face.height) / static_cast<float>(picInfo->picHeight);
                result->faces[i].confidence = 1.0f; // For Haar classifier, set to 1.0 as an example
            }
        } else {
            result->faces = nullptr;
        }

        return 0; // Successfully detected
    }
    catch (const cv::Exception& e) {
        LOG_ERROR("Error detecting faces: {}", e.what());
        
        // Clean up on error
        if (result->faces) {
            delete[] result->faces;
            result->faces = nullptr;
        }
        result->faceCount = 0;
        
        return -1;
    }
    catch (const std::exception& e) {
        LOG_ERROR("General error during face detection: {}", e.what());
        
        // Clean up on error
        if (result->faces) {
            delete[] result->faces;
            result->faces = nullptr;
        }
        result->faceCount = 0;
        
        return -1;
    }
    catch (...) {
        LOG_ERROR("Unknown error occurred during face detection");
        
        // Clean up on error
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
}