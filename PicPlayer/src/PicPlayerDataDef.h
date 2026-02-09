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

// 输入
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

#ifdef __cplusplus
}
#endif
#endif // PICPLAYERDATADEF_H

