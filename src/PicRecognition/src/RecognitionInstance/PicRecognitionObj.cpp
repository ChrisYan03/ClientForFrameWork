#include "PicRecognitionObj.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <algorithm>
#include "LogUtil.h"
#include <fstream>

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
    , use_dnn(false)
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

        // 尝试加载DNN模型文件
        std::vector<std::string> dnnConfigPaths = {
            exe_dir + "/face_detector/opencv_face_detector.pbtxt",
            exe_dir + "/face_detector/deploy.prototxt",
            exe_dir + "/face_detector/opencv_face_detector_uint8.pbtxt"
        };
        
        std::vector<std::string> dnnModelPaths = {
            exe_dir + "/face_detector/opencv_face_detector_uint8.pb",
            exe_dir + "/face_detector/res10_300x300_ssd_iter_140000.caffemodel"
        };

        bool dnnConfigLoaded = false;
        bool dnnModelLoaded = false;
        std::string configPath, modelPath;

        // 查找配置文件
        for (const auto& path : dnnConfigPaths) {
            std::ifstream file(path);
            if (file.is_open()) {
                configPath = path;
                dnnConfigLoaded = true;
                LOG_INFO("Found DNN config file: {}", path);
                file.close();
                break;
            }
        }

        // 查找模型文件
        for (const auto& path : dnnModelPaths) {
            std::ifstream file(path);
            if (file.is_open()) {
                modelPath = path;
                dnnModelLoaded = true;
                LOG_INFO("Found DNN model file: {}", path);
                file.close();
                break;
            }
        }

        // 如果都找到了，尝试加载DNN模型
        if (dnnConfigLoaded && dnnModelLoaded) {
            try {
                // 判断是哪种模型格式
                if (modelPath.find(".pb") != std::string::npos && configPath.find(".pbtxt") != std::string::npos) {
                    // TensorFlow模型
                    net = cv::dnn::readNetFromTensorflow(modelPath, configPath);
                } else if (modelPath.find(".caffemodel") != std::string::npos && configPath.find(".prototxt") != std::string::npos) {
                    // Caffe模型
                    net = cv::dnn::readNetFromCaffe(configPath, modelPath);
                }

                if (!net.empty()) {
                    use_dnn = true;
                    LOG_INFO("Successfully loaded DNN face detection model");
                } else {
                    LOG_WARN("Failed to load DNN model, falling back to Haar cascades");
                }
            } catch (const cv::Exception& e) {
                LOG_WARN("DNN model loading failed: {}, falling back to Haar cascades", e.what());
            }
        }

        // 如果DNN模型未成功加载，继续使用传统方法
        if (!use_dnn) {
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
        }

        LOG_INFO("Face recognition initialized successfully, using DNN: {}", use_dnn);
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

// 使用DNN进行人脸检测
std::vector<cv::Rect> FaceRecognitionManager::detectFacesWithDNN(const cv::Mat& image) {
    std::vector<cv::Rect> faces;
    
    try {
        LOG_DEBUG("Starting DNN face detection, image size: {}x{}", image.cols, image.rows);
        LOG_DEBUG("Net is empty: {}", net.empty());
        if(net.empty()) {
            LOG_ERROR("DNN network is empty, cannot perform detection");
            return faces;
        }

        // 创建blob - 对于SSD模型，通常输入尺寸为300x300
        cv::Mat blob = cv::dnn::blobFromImage(image, 1.0, cv::Size(300, 300), cv::Scalar(104.0, 177.0, 123.0), false);
        LOG_DEBUG("Blob created with size: {}x{}x{}x{}", blob.size[0], blob.size[1], blob.size[2], blob.size[3]);
        net.setInput(blob);
        // 尝试不同的输出层名称，这是OpenCV DNN人脸检测模型的标准输出层名称
        std::vector<cv::String> outBlobNames = {"detection_out", "detection_out_final", "detection_output", "output"};
        cv::Mat detections;
        bool detectionSuccess = false;
        
        // 尝试标准输出层名称
        for (const auto& outName : outBlobNames) {
            try {
                detections = net.forward(outName);
                LOG_DEBUG("Successfully got detections using output layer: {}, dims: {}", outName, detections.dims);
                if (detections.total() > 0) {
                    detectionSuccess = true;
                    break;
                }
            } catch (const cv::Exception& e) {
                LOG_DEBUG("Failed to get detections using output layer '{}': {}", outName, e.what());
            }
        }

        // 如果仍然失败，尝试直接forward
        if (!detectionSuccess) {
            try {
                detections = net.forward();
                LOG_DEBUG("Got detections using default forward, dims: {}", detections.dims);
                if (detections.total() > 0) {
                    detectionSuccess = true;
                }
            } catch (const cv::Exception& e) {
                LOG_ERROR("Failed to get detections using default forward: {}", e.what());
            }
        }

        if (!detectionSuccess) {
            LOG_ERROR("Could not retrieve detections from network");
            return faces;
        }

        LOG_DEBUG("Detections matrix dimensions: {}", detections.dims);
        if (detections.dims == 4 && detections.size[0] == 1 && detections.size[1] == 1 && detections.size[2] > 0 && detections.size[3] >= 7) {
            // Caffe模型通常输出格式为 [1, 1, num_detections, 7]
            LOG_DEBUG("Detected Caffe-style output format: 1x1xNx7, num_detections: {}", detections.size[2]);
            for (int i = 0; i < detections.size[2]; i++) {
                float confidence = detections.ptr<float>(0, 0, i)[2];
                float x_start = detections.ptr<float>(0, 0, i)[3];
                float y_start = detections.ptr<float>(0, 0, i)[4];
                float x_end = detections.ptr<float>(0, 0, i)[5];
                float y_end = detections.ptr<float>(0, 0, i)[6];
                
                LOG_DEBUG("Detection {}: confidence={:.3f}, coords=({:.3f},{:.3f})-({:.3f},{:.3f})", 
                          i, confidence, x_start, y_start, x_end, y_end);
                
                // 设置置信度阈值
                if (confidence > 0.5) {  // 可根据需要调整阈值
                    int x = static_cast<int>(x_start * image.cols);
                    int y = static_cast<int>(y_start * image.rows);
                    int w = static_cast<int>(x_end * image.cols - x);
                    int h = static_cast<int>(y_end * image.rows - y);
                    
                    // 确保边界不超出图像范围
                    x = std::max(0, x);
                    y = std::max(0, y);
                    w = std::min(image.cols - x, w);
                    h = std::min(image.rows - y, h);
                    
                    LOG_DEBUG("Converted coordinates: x={}, y={}, w={}, h={}", x, y, w, h);
                    
                    // 过滤掉过小的人脸
                    if (w >= 20 && h >= 20) {
                        LOG_DEBUG("Adding face detection: ({},{})-({},{})", x, y, x+w, y+h);
                        faces.push_back(cv::Rect(x, y, w, h));
                    } else {
                        LOG_DEBUG("Skipping small face: w={}, h={} (less than 20)", w, h);
                    }
                } else {
                    LOG_DEBUG("Skipping low confidence detection: {:.3f} (threshold: 0.5)", confidence);
                }
            }
        } else if (detections.dims == 3 && detections.size[0] == 1 && detections.size[1] > 0 && detections.size[2] >= 7) {
            // 另一种可能的格式 [1, num_detections, 7]
            LOG_DEBUG("Detected alternative output format: 1xNx7, num_detections: {}", detections.size[1]);
            for (int i = 0; i < detections.size[1]; i++) {
                float confidence = detections.ptr<float>(0, i)[2];
                float x_start = detections.ptr<float>(0, i)[3];
                float y_start = detections.ptr<float>(0, i)[4];
                float x_end = detections.ptr<float>(0, i)[5];
                float y_end = detections.ptr<float>(0, i)[6];
                
                LOG_DEBUG("Detection {}: confidence={:.3f}, coords=({:.3f},{:.3f})-({:.3f},{:.3f})", 
                          i, confidence, x_start, y_start, x_end, y_end);
                
                // 设置置信度阈值
                if (confidence > 0.7) {  // 可据需要调整阈值
                    int x = static_cast<int>(x_start * image.cols);
                    int y = static_cast<int>(y_start * image.rows);
                    int w = static_cast<int>(x_end * image.cols - x);
                    int h = static_cast<int>(y_end * image.rows - y);
                    
                    // 确保边界不超出图像范围
                    x = std::max(0, x);
                    y = std::max(0, y);
                    w = std::min(image.cols - x, w);
                    h = std::min(image.rows - y, h);
                    
                    LOG_DEBUG("Converted coordinates: x={}, y={}, w={}, h={}", x, y, w, h);
                    
                    // 过滤掉过小的人脸
                    if (w >= 20 && h >= 20) {
                        LOG_DEBUG("Adding face detection: ({},{})-({},{})", x, y, x+w, y+h);
                        faces.push_back(cv::Rect(x, y, w, h));
                    } else {
                        LOG_DEBUG("Skipping small face: w={}, h={} (less than 20)", w, h);
                    }
                } 
            }
        } else if (detections.dims == 2 && detections.size[0] > 0 && detections.size[1] >= 7) {
            // 标准格式：每一行是 [image_id, class_id, confidence, x, y, w, h]
            LOG_DEBUG("Detected standard output format: MxN, rows: {}, cols: {}", detections.size[0], detections.size[1]);
            for (int i = 0; i < detections.size[0]; i++) {
                float confidence = detections.ptr<float>(i)[2];
                float x_start = detections.ptr<float>(i)[3];
                float y_start = detections.ptr<float>(i)[4];
                float x_end = detections.ptr<float>(i)[5];
                float y_end = detections.ptr<float>(i)[6];
                
                LOG_DEBUG("Detection {}: confidence={:.3f}, coords=({:.3f},{:.3f})-({:.3f},{:.3f})", 
                          i, confidence, x_start, y_start, x_end, y_end);
                
                // 设置置信度阈值
                if (confidence > 0.5) {  // 可根据需要调整阈值
                    int x = static_cast<int>(x_start * image.cols);
                    int y = static_cast<int>(y_start * image.rows);
                    int w = static_cast<int>(x_end * image.cols - x);
                    int h = static_cast<int>(y_end * image.rows - y);
                    
                    // 确保边界不超出图像范围
                    x = std::max(0, x);
                    y = std::max(0, y);
                    w = std::min(image.cols - x, w);
                    h = std::min(image.rows - y, h);
                    
                    LOG_DEBUG("Converted coordinates: x={}, y={}, w={}, h={}", x, y, w, h);
                    
                    // 过滤掉过小的人脸
                    if (w >= 20 && h >= 20) {
                        LOG_DEBUG("Adding face detection: ({},{})-({},{})", x, y, x+w, y+h);
                        faces.push_back(cv::Rect(x, y, w, h));
                    } else {
                        LOG_DEBUG("Skipping small face: w={}, h={} (less than 20)", w, h);
                    }
                }
            }
        } else {
            LOG_ERROR("Unexpected detection matrix format: dims={}, sizes:", detections.dims);
            for (int i = 0; i < detections.dims; i++) {
                LOG_ERROR("dim[{}]={}", i, detections.size[i]);
            }
        }
    } catch (const cv::Exception& e) {
        LOG_ERROR("DNN face detection error: {}", e.what());
    }
    
    LOG_DEBUG("DNN face detection finished, found {} faces", faces.size());
    return faces;
}

int FaceRecognitionManager::detectFacesInRgba(PicShowInfo* picInfo, FaceDetectionResult* result)
{
    if (!picInfo || !result) {
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

        if (use_dnn) {
            LOG_DEBUG("Using DNN for face detection");
            // 使用DNN进行人脸检测 - DNN模型通常需要BGR图像
            cv::Mat dnnInputImg = bgrMat.clone(); // 使用彩色图像而不是灰度图
            // 实际上，对于人脸检测，我们应使用彩色BGR图像
            cv::Mat colorImg;
            cv::cvtColor(rgbaMat, colorImg, cv::COLOR_RGBA2BGR);
            std::vector<cv::Rect> dnnFaces = detectFacesWithDNN(colorImg);
            LOG_DEBUG("DNN returned {} faces", dnnFaces.size());
            for (const auto& face : dnnFaces) {
                float quality = calculateFaceQuality(grayMat, face, false);
                float confidence = 0.8f + 0.2f * quality; // DNN通常更准确
                allFaces.emplace_back(face, quality, confidence, false, 0);
            }
        } else {
            LOG_DEBUG("Using Haar cascade for face detection");
            // 传统方法：正面人脸检测
            if (face_cascade) {
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

        // 输出人脸检测信息日志
        LOG_DEBUG("Face detection completed: {} faces detected for image ID: {}", 
                  result->faceCount, picInfo->imageId);
        for (int i = 0; i < result->faceCount; ++i) {
            LOG_DEBUG("Face {}: x={:.3f}, y={:.3f}, width={:.3f}, height={:.3f}, confidence={:.3f}", 
                      i, result->faces[i].x, result->faces[i].y, result->faces[i].width, 
                      result->faces[i].height, result->faces[i].confidence);
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
    net = cv::dnn::Net(); // 清空网络
}