#ifndef PICPLAYERDATADEF_H
#define PICPLAYERDATADEF_H

#include "PicPlayerGlobal.h"
#ifdef _WIN32
    #include <stddef.h>
    #include <stdint.h>
    #include <string>
#else
    #include <cstddef>
    #include <cstdint>
    #include <cstdlib>
    #include <cstring>
#endif


#ifdef __cplusplus
extern "C" {
#endif
typedef void(PICPLAYER_CALL *PlayerMsgCallback)(int handle, int iMsg, void* pData, void* pUser);

#ifdef __APPLE__
using Window_ShowID = decltype(sizeof(void*));
#else
typedef size_t Window_ShowID;
#endif

#define IMAGE_ID_LEN 128

// 输出
enum CallbackType
{
    Callback_RemovePicId,
    Callback_ShowPicId,
};

// 输入 图片数据
struct PicShowInfo
{
    uint32_t picReadTime;
    uint32_t picWidth;
    uint32_t picHeight;
    char imageId[IMAGE_ID_LEN];
    char* imageRgbaData;
    size_t imageRgbaLen;

    PicShowInfo()
        : picReadTime(0), picWidth(0), picHeight()
        , imageRgbaData(nullptr), imageRgbaLen(0)
    {
        memset(imageId, 0, IMAGE_ID_LEN);
    }
    // 拷贝构造函数
    PicShowInfo(const PicShowInfo& other)
        : picReadTime(other.picReadTime), picWidth(other.picWidth), picHeight(other.picHeight)
        , imageRgbaData(nullptr), imageRgbaLen(other.imageRgbaLen)
    {
        memset(imageId, 0, IMAGE_ID_LEN);
        std::strncpy(imageId, other.imageId, IMAGE_ID_LEN);
        if (other.imageRgbaData) {
            imageRgbaData = static_cast<char*>(malloc(other.imageRgbaLen));
            if (imageRgbaData) {
                std::memcpy(imageRgbaData, other.imageRgbaData, other.imageRgbaLen);
            }
        }
    }
    // 赋值运算符
    PicShowInfo& operator=(const PicShowInfo& other) {
        if (this != &other) {
            clear();
            picReadTime = other.picReadTime;
            picWidth = other.picWidth;
            picHeight = other.picHeight;
            imageRgbaLen = other.imageRgbaLen;
            memset(imageId, 0, IMAGE_ID_LEN);
            std::strncpy(imageId, other.imageId, IMAGE_ID_LEN);
            if (other.imageRgbaData) {
                imageRgbaData = static_cast<char*>(malloc(other.imageRgbaLen));
                if (imageRgbaData) {
                    std::memcpy(imageRgbaData, other.imageRgbaData, other.imageRgbaLen);
                }
            }
        }
        return *this;
    }

    ~PicShowInfo() {
        clear();
    }

    // 辅助函数：清除并释放内存
    void clear() {
        free(imageRgbaData);
        imageRgbaData = nullptr;
        imageRgbaLen = 0;
    }
};

// 输入 图片识别数据
struct FaceInfo
{
    float x;
    float y;
    float width;
    float height;
    float confidence; // Confidence level
    int age;          // Age of the person detected
    char* faceImageData; // Cropped face image data in RGBA format
    size_t faceImageLength; // Length of the face image data
    int faceImageWidth;  // Width of the cropped face image
    int faceImageHeight; // Height of the cropped face image

    FaceInfo()
        : x(0.0f), y(0.0f), width(0.0f), height(0.0f), confidence(0.0f), age(-1)
        , faceImageData(nullptr), faceImageLength(0), faceImageWidth(0), faceImageHeight(0)
    {}
    
    // Copy constructor
    FaceInfo(const FaceInfo& other)
        : x(other.x), y(other.y), width(other.width), height(other.height), confidence(other.confidence), age(other.age)
        , faceImageWidth(other.faceImageWidth), faceImageHeight(other.faceImageHeight), faceImageLength(other.faceImageLength)
    {
        if (other.faceImageData && other.faceImageLength > 0) {
            faceImageData = static_cast<char*>(malloc(other.faceImageLength));
            if (faceImageData) {
                std::memcpy(faceImageData, other.faceImageData, other.faceImageLength);
            }
        } else {
            faceImageData = nullptr;
        }
    }
    
    // Assignment operator
    FaceInfo& operator=(const FaceInfo& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
            width = other.width;
            height = other.height;
            confidence = other.confidence;
            age = other.age;
            
            // Free existing image data
            if (faceImageData) {
                free(faceImageData);
            }
            
            faceImageWidth = other.faceImageWidth;
            faceImageHeight = other.faceImageHeight;
            faceImageLength = other.faceImageLength;
            
            if (other.faceImageData && other.faceImageLength > 0) {
                faceImageData = static_cast<char*>(malloc(other.faceImageLength));
                if (faceImageData) {
                    std::memcpy(faceImageData, other.faceImageData, other.faceImageLength);
                }
            } else {
                faceImageData = nullptr;
            }
        }
        return *this;
    }
    
    // Destructor
    ~FaceInfo() {
        if (faceImageData) {
            free(faceImageData);
            faceImageData = nullptr;
        }
    }
};

struct FaceDetectionResult
{
    char imageId[IMAGE_ID_LEN];
    int faceCount;
    FaceInfo* faces;

    // Default constructor
    FaceDetectionResult()
        : faceCount(0), faces(nullptr)
    {
        memset(imageId, 0, IMAGE_ID_LEN);
    }
    
    // Constructor with parameters
    FaceDetectionResult(const char* id, int count, FaceInfo* faceArray)
        : faceCount(count)
    {
        memset(imageId, 0, IMAGE_ID_LEN);
        if (id) {
            strncpy(imageId, id, IMAGE_ID_LEN - 1);
        }
        faces = nullptr;
        if (faceArray && count > 0) {
            faces = new FaceInfo[count];
            for (int i = 0; i < count; ++i) {
                faces[i] = faceArray[i];
            }
        }
    }
    
    // Copy constructor
    FaceDetectionResult(const FaceDetectionResult& other)
        : faceCount(other.faceCount)
    {
        memset(imageId, 0, IMAGE_ID_LEN);
        strncpy(imageId, other.imageId, IMAGE_ID_LEN - 1);
        
        faces = nullptr;
        if (other.faces && other.faceCount > 0) {
            faces = new FaceInfo[other.faceCount];
            for (int i = 0; i < other.faceCount; ++i) {
                faces[i] = other.faces[i];
            }
        }
    }
    
    // Assignment operator
    FaceDetectionResult& operator=(const FaceDetectionResult& other) {
        if (this != &other) {
            // Free existing memory
            delete[] faces;
            
            faceCount = other.faceCount;
            memset(imageId, 0, IMAGE_ID_LEN);
            strncpy(imageId, other.imageId, IMAGE_ID_LEN - 1);
            
            faces = nullptr;
            if (other.faces && other.faceCount > 0) {
                faces = new FaceInfo[other.faceCount];
                for (int i = 0; i < other.faceCount; ++i) {
                    faces[i] = other.faces[i];
                }
            }
        }
        return *this;
    }
    
    // Destructor
    ~FaceDetectionResult() {
        delete[] faces;
        faces = nullptr;
    }
};

#ifdef __cplusplus
}
#endif
#endif // PICPLAYERDATADEF_H

