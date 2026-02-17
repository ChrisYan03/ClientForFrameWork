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
    int age;  // New field for storing detected age
    
    FaceQuality(const cv::Rect& r, float score, float conf, bool profile = false, int type = 0, int a = -1)
        : rect(r), qualityScore(score), confidence(conf), isProfile(profile), detectionType(type), age(a) {}
    
    bool operator<(const FaceQuality& other) const {
        return qualityScore > other.qualityScore;
    }
};

FaceRecognitionManager::FaceRecognitionManager()
    : face_cascade(nullptr)
    , profile_face_cascade(nullptr)
    , faceNet()               // 初始化人脸检测网络
    , ageNet()                // 初始化年龄估计网络
    , use_dnn(false)
    , use_age_estimation(false) // 默认不启用年龄估计
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
                    faceNet = cv::dnn::readNetFromTensorflow(modelPath, configPath);
                } else if (modelPath.find(".caffemodel") != std::string::npos && configPath.find(".prototxt") != std::string::npos) {
                    // Caffe模型
                    faceNet = cv::dnn::readNetFromCaffe(configPath, modelPath);
                }

                if (!faceNet.empty()) {
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

        // 尝试加载年龄估计模型
        std::vector<std::string> ageConfigPaths = {
            exe_dir + "/face_detector/age_deploy.prototxt",
            exe_dir + "/age_model/age_deploy.prototxt",
            exe_dir + "/models/age_deploy.prototxt",
            exe_dir + "/data/models/age_deploy.prototxt"
        };

        std::vector<std::string> ageModelPaths = {
            exe_dir + "/face_detector/age_net.caffemodel",
            exe_dir + "/age_model/age_net.caffemodel",
            exe_dir + "/models/age_net.caffemodel",
            exe_dir + "/data/models/age_net.caffemodel"
        };

        std::string ageConfigPath, ageModelPath;
        bool ageConfigFound = false;
        bool ageModelFound = false;

        // 查找年龄模型配置文件
        for (const auto& path : ageConfigPaths) {
            std::ifstream file(path);
            if (file.is_open()) {
                ageConfigPath = path;
                ageConfigFound = true;
                LOG_INFO("Found age model config: {}", path);
                file.close();
                break;
            }
        }

        // 查找年龄模型文件
        for (const auto& path : ageModelPaths) {
            std::ifstream file(path);
            if (file.is_open()) {
                ageModelPath = path;
                ageModelFound = true;
                LOG_INFO("Found age model: {}", path);
                file.close();
                break;
            }
        }

        // 如果年龄模型文件都找到了，尝试加载
        if (ageConfigFound && ageModelFound) {
            try {
                ageNet = cv::dnn::readNetFromCaffe(ageConfigPath, ageModelPath);
                if (!ageNet.empty()) {
                    use_age_estimation = true;
                    LOG_INFO("Successfully loaded age estimation model");
                } else {
                    LOG_WARN("Failed to load age estimation model");
                }
            } catch (const cv::Exception& e) {
                LOG_WARN("Age estimation model loading failed: {}", e.what());
            }
        } else {
            LOG_WARN("Age estimation model files not found, age detection will be disabled");
        }

        LOG_INFO("Face recognition initialized successfully, using DNN: {}, age estimation: {}", use_dnn, use_age_estimation);
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

// 年龄估计函数
int FaceRecognitionManager::estimateAge(const cv::Mat& faceImage) {
    // 如果没有加载年龄估计模型，直接返回-1
    if (!use_age_estimation || ageNet.empty()) {
        return -1;
    }
    
    try {
        // 预处理人脸图像用于年龄估计
        cv::Mat inputBlob = cv::dnn::blobFromImage(faceImage, 1.0, cv::Size(227, 227), 
                                                  cv::Scalar(78.42633776, 87.76891437, 114.89584775));
        
        // 设置输入到年龄估计模型
        ageNet.setInput(inputBlob);
        
        // 运行前向传播获取年龄预测
        cv::Mat agePreds = ageNet.forward();
        
        // 定义年龄组
        std::vector<std::string> ageList = {"(0-2)", "(4-6)", "(8-12)", "(15-20)", 
                                          "(25-32)", "(38-43)", "(48-53)", "(60-100)"};
        
        // 获取最高概率的索引
        cv::Point maxLoc;
        double maxVal;
        minMaxLoc(agePreds, NULL, &maxVal, NULL, &maxLoc);
        int ageIdx = maxLoc.x;
        
        // 返回估算的年龄（使用年龄范围的中间值）
        std::string ageRange = ageList[ageIdx];
        LOG_DEBUG("Age prediction: range={}, confidence={:.3f}", ageRange, maxVal);
        if (ageRange == "(0-2)") return 1;
        else if (ageRange == "(4-6)") return 4;
        else if (ageRange == "(8-12)") return 10;
        else if (ageRange == "(15-20)") return 17;
        else if (ageRange == "(25-32)") return 28;
        else if (ageRange == "(38-43)") return 40;
        else if (ageRange == "(48-53)") return 50;
        else if (ageRange == "(60-100)") return 70;
        
        return -1; // 未知年龄
        
    } catch (const cv::Exception& e) {
        LOG_WARN("Age estimation failed: {}, skipping age detection", e.what());
        return -1;
    }
}

// 使用DNN进行人脸检测
std::vector<cv::Rect> FaceRecognitionManager::detectFacesWithDNN(const cv::Mat& image) {
    std::vector<cv::Rect> faces;
    
    try {
        LOG_DEBUG("Starting DNN face detection, image size: {}x{}", image.cols, image.rows);
        LOG_DEBUG("Net is empty: {}", faceNet.empty());
        if(faceNet.empty()) {
            LOG_ERROR("DNN network is empty, cannot perform detection");
            return faces;
        }

        // 创建blob - 对于SSD模型，通常输入尺寸为300x300
        cv::Mat blob = cv::dnn::blobFromImage(image, 1.0, cv::Size(300, 300), cv::Scalar(104.0, 177.0, 123.0), false);
        LOG_DEBUG("Blob created with size: {}x{}x{}x{}", blob.size[0], blob.size[1], blob.size[2], blob.size[3]);
        faceNet.setInput(blob);
        // 尝试不同的输出层名称，这是OpenCV DNN人脸检测模型的标准输出层名称
        std::vector<cv::String> outBlobNames = {"detection_out", "detection_out_final", "detection_output", "output"};
        cv::Mat detections;
        bool detectionSuccess = false;
        
        // 尝试标准输出层名称
        for (const auto& outName : outBlobNames) {
            try {
                detections = faceNet.forward(outName);
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
                detections = faceNet.forward();
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

                // 降低置信度阈值以检测更模糊或遮挡的人脸
                if (confidence > 0.3) {  // 从0.5降低到0.3，使检测更敏感
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
                    if (w >= 15 && h >= 15) {  // 从20减少到15，可以检测更小的人脸
                        LOG_DEBUG("Adding face detection: ({},{})-({},{})", x, y, x+w, y+h);
                        faces.push_back(cv::Rect(x, y, w, h));
                    } else {
                        LOG_DEBUG("Skipping small face: w={}, h={} (less than 15)", w, h);
                    }
                } else {
                    LOG_DEBUG("Skipping low confidence detection: {:.3f} (threshold: 0.3)", confidence);
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
                
                // 降低置信度阈值以检测更模糊或遮挡的人脸
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
                    if (w >= 15 && h >= 15) {  // 从20减少到15，可以检测更小的人脸
                        LOG_DEBUG("Adding face detection: ({},{})-({},{})", x, y, x+w, y+h);
                        faces.push_back(cv::Rect(x, y, w, h));
                    } else {
                        LOG_DEBUG("Skipping small face: w={}, h={} (less than 15)", w, h);
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
                
                // 降低置信度阈值以检测更模糊或遮挡的人脸
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
                    if (w >= 15 && h >= 15) {  // 从20减少到15，可以检测更小的人脸
                        LOG_DEBUG("Adding face detection: ({},{})-({},{})", x, y, x+w, y+h);
                        faces.push_back(cv::Rect(x, y, w, h));
                    } else {
                        LOG_DEBUG("Skipping small face: w={}, h={} (less than 15)", w, h);
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
            
            // 对于可能包含遮挡或角度不佳的人脸，使用更敏感的检测参数
            std::vector<cv::Rect> dnnFaces = detectFacesWithDNN(colorImg);
            LOG_DEBUG("DNN returned {} faces", dnnFaces.size());
            for (const auto& face : dnnFaces) {
                float quality = calculateFaceQuality(grayMat, face, false);
                float confidence = 0.8f + 0.2f * quality; // DNN通常更准确
                
                // 提取人脸区域进行年龄估计
                cv::Mat faceRegion = colorImg(face);
                int estimatedAge = estimateAge(faceRegion);
                
                allFaces.emplace_back(face, quality, confidence, false, 0, estimatedAge);
            }
        } else {
            LOG_DEBUG("Using Haar cascade for face detection");
            // 传统方法：正面人脸检测
            if (face_cascade) {
                std::vector<cv::Rect> frontalFaces;
                // 使用更敏感的检测参数，以检测可能被遮挡或角度不佳的人脸
                face_cascade->detectMultiScale(grayMat, frontalFaces, 1.08, 5, 0, cv::Size(20, 20));
                
                for (const auto& face : frontalFaces) {
                    float quality = calculateFaceQuality(grayMat, face, false);
                    float confidence = 0.8f + 0.2f * quality;
                    
                    // 提取人脸区域进行年龄估计（使用BGR图像）
                    cv::Mat faceRegion = bgrMat(face);
                    int estimatedAge = estimateAge(faceRegion);
                    
                    allFaces.emplace_back(face, quality, confidence, false, 0, estimatedAge);
                }

                // 侧脸检测 - 简化版本
                if (profile_face_cascade) {
                    // 左侧脸
                    std::vector<cv::Rect> leftFaces;
                    profile_face_cascade->detectMultiScale(grayMat, leftFaces, 1.08, 5, 0, cv::Size(20, 20));
                    for (const auto& face : leftFaces) {
                        float quality = calculateFaceQuality(grayMat, face, true) * 0.85f;
                        float confidence = 0.6f + 0.2f * quality;
                        
                        // 提取人脸区域进行年龄估计（使用BGR图像）
                        cv::Mat faceRegion = bgrMat(face);
                        int estimatedAge = estimateAge(faceRegion);
                        
                        allFaces.emplace_back(face, quality, confidence, true, 1, estimatedAge);
                    }
                    
                    // 右侧脸（图像翻转）
                    cv::Mat flipped;
                    cv::flip(grayMat, flipped, 1);
                    cv::Mat flippedColor;
                    cv::flip(bgrMat, flippedColor, 1);
                    std::vector<cv::Rect> rightFaces;
                    profile_face_cascade->detectMultiScale(flipped, rightFaces, 1.08, 5, 0, cv::Size(20, 20));
                    for (auto& face : rightFaces) {
                        face.x = grayMat.cols - face.x - face.width;
                        float quality = calculateFaceQuality(grayMat, face, true) * 0.85f;
                        float confidence = 0.6f + 0.2f * quality;
                        
                        // 从翻转后的彩色图像中提取人脸区域进行年龄估计
                        cv::Rect adjustedFace(face.x, face.y, face.width, face.height);
                        cv::Mat faceRegion = flippedColor(adjustedFace);
                        int estimatedAge = estimateAge(faceRegion);
                        
                        allFaces.emplace_back(face, quality, confidence, true, 2, estimatedAge);
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
                cv::Rect unionRect = face.rect | existing.rect;
                if (unionRect.area() > 0 && static_cast<float>(intersection.area()) / unionRect.area() > 0.3f) {
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
                result->faces[i].age = fq.age;  // Assign the detected age
            }
        }

        // 输出人脸检测信息日志
        LOG_DEBUG("Face detection completed: {} faces detected for image ID: {}", 
                  result->faceCount, picInfo->imageId);
        for (int i = 0; i < result->faceCount; ++i) {
            LOG_DEBUG("Face {}: x={:.3f}, y={:.3f}, width={:.3f}, height={:.3f}, confidence={:.3f}, age={}", 
                      i, result->faces[i].x, result->faces[i].y, result->faces[i].width, 
                      result->faces[i].height, result->faces[i].confidence, result->faces[i].age);
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
    faceNet = cv::dnn::Net(); // 清空人脸检测网络
    ageNet = cv::dnn::Net();  // 清空年龄估计网络
}