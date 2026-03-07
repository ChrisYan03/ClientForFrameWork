#include "IPlayerDataDef.h"
#include <cstring>
#include <cstdlib>

namespace ComponentInterface {

// ==============================================================================
// FaceInfo Implementation
// ==============================================================================
FaceInfo::~FaceInfo() {
    clear();
}

FaceInfo::FaceInfo(FaceInfo&& other) noexcept
    : x(other.x)
    , y(other.y)
    , width(other.width)
    , height(other.height)
    , faceImageData(other.faceImageData)
    , faceImageLength(other.faceImageLength)
    , faceImageWidth(other.faceImageWidth)
    , faceImageHeight(other.faceImageHeight)
{
    other.faceImageData = nullptr;
    other.faceImageLength = 0;
    other.faceImageWidth = 0;
    other.faceImageHeight = 0;
}

FaceInfo& FaceInfo::operator=(FaceInfo&& other) noexcept {
    if (this != &other) {
        clear();

        x = other.x;
        y = other.y;
        width = other.width;
        height = other.height;
        faceImageData = other.faceImageData;
        faceImageLength = other.faceImageLength;
        faceImageWidth = other.faceImageWidth;
        faceImageHeight = other.faceImageHeight;

        other.faceImageData = nullptr;
        other.faceImageLength = 0;
        other.faceImageWidth = 0;
        other.faceImageHeight = 0;
    }
    return *this;
}

void FaceInfo::clear() {
    if (faceImageData) {
        free(faceImageData);
        faceImageData = nullptr;
    }
    faceImageLength = 0;
    faceImageWidth = 0;
    faceImageHeight = 0;
}

// ==============================================================================
// FaceDetectionResult Implementation
// ==============================================================================
FaceDetectionResult::FaceDetectionResult()
    : faces(nullptr)
    , faceCount(0)
{
    std::memset(imageId, 0, IMAGE_ID_LEN);
}

FaceDetectionResult::~FaceDetectionResult() {
    clear();
}

FaceDetectionResult::FaceDetectionResult(FaceDetectionResult&& other) noexcept
    : faces(other.faces)
    , faceCount(other.faceCount)
{
    std::memcpy(imageId, other.imageId, IMAGE_ID_LEN);

    other.faces = nullptr;
    other.faceCount = 0;
    std::memset(other.imageId, 0, IMAGE_ID_LEN);
}

FaceDetectionResult& FaceDetectionResult::operator=(FaceDetectionResult&& other) noexcept {
    if (this != &other) {
        clear();

        faces = other.faces;
        faceCount = other.faceCount;
        std::memcpy(imageId, other.imageId, IMAGE_ID_LEN);

        other.faces = nullptr;
        other.faceCount = 0;
        std::memset(other.imageId, 0, IMAGE_ID_LEN);
    }
    return *this;
}

void FaceDetectionResult::clear() {
    if (faces) {
        for (int i = 0; i < faceCount; ++i) {
            faces[i].clear();
        }
        free(faces);
        faces = nullptr;
    }
    faceCount = 0;
    std::memset(imageId, 0, IMAGE_ID_LEN);
}

// ==============================================================================
// PicShowInfo Implementation
// ==============================================================================
PicShowInfo::PicShowInfo()
    : picReadTime(0)
    , picWidth(0)
    , picHeight(0)
    , imageRgbaData(nullptr)
    , imageRgbaLen(0)
{
    std::memset(imageId, 0, IMAGE_ID_LEN);
}

PicShowInfo::~PicShowInfo() {
    clear();
}

PicShowInfo::PicShowInfo(const PicShowInfo& other)
    : picReadTime(other.picReadTime)
    , picWidth(other.picWidth)
    , picHeight(other.picHeight)
    , imageRgbaData(nullptr)
    , imageRgbaLen(other.imageRgbaLen)
{
    std::memset(imageId, 0, IMAGE_ID_LEN);
    std::strncpy(imageId, other.imageId, IMAGE_ID_LEN);

    if (other.imageRgbaData) {
        imageRgbaData = static_cast<char*>(malloc(imageRgbaLen));
        if (imageRgbaData) {
            std::memcpy(imageRgbaData, other.imageRgbaData, imageRgbaLen);
        }
    }
}

PicShowInfo& PicShowInfo::operator=(const PicShowInfo& other) {
    if (this != &other) {
        clear();

        picReadTime = other.picReadTime;
        picWidth = other.picWidth;
        picHeight = other.picHeight;
        imageRgbaLen = other.imageRgbaLen;
        std::memset(imageId, 0, IMAGE_ID_LEN);
        std::strncpy(imageId, other.imageId, IMAGE_ID_LEN);

        if (other.imageRgbaData) {
            imageRgbaData = static_cast<char*>(malloc(imageRgbaLen));
            if (imageRgbaData) {
                std::memcpy(imageRgbaData, other.imageRgbaData, imageRgbaLen);
            }
        }
    }
    return *this;
}

PicShowInfo::PicShowInfo(PicShowInfo&& other) noexcept
    : picReadTime(other.picReadTime)
    , picWidth(other.picWidth)
    , picHeight(other.picHeight)
    , imageRgbaData(other.imageRgbaData)
    , imageRgbaLen(other.imageRgbaLen)
{
    std::memcpy(imageId, other.imageId, IMAGE_ID_LEN);

    other.imageRgbaData = nullptr;
    other.imageRgbaLen = 0;
    std::memset(other.imageId, 0, IMAGE_ID_LEN);
}

PicShowInfo& PicShowInfo::operator=(PicShowInfo&& other) noexcept {
    if (this != &other) {
        clear();

        picReadTime = other.picReadTime;
        picWidth = other.picWidth;
        picHeight = other.picHeight;
        imageRgbaData = other.imageRgbaData;
        imageRgbaLen = other.imageRgbaLen;
        std::memcpy(imageId, other.imageId, IMAGE_ID_LEN);

        other.imageRgbaData = nullptr;
        other.imageRgbaLen = 0;
        std::memset(other.imageId, 0, IMAGE_ID_LEN);
    }
    return *this;
}

void PicShowInfo::clear() {
    if (imageRgbaData) {
        free(imageRgbaData);
        imageRgbaData = nullptr;
    }
    imageRgbaLen = 0;
}

} // namespace ComponentInterface