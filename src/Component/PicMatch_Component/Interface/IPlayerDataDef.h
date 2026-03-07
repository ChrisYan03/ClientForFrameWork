#pragma once

#include <memory>
#include <vector>
#include <string>
#include <cstdint>

namespace ComponentInterface {

// ==============================================================================
// Constants
// ==============================================================================
constexpr size_t IMAGE_ID_LEN = 64;

// ==============================================================================
// Forward Declarations
// ==============================================================================
struct FaceInfo;
struct FaceDetectionResult;
struct PicShowInfo;

// ==============================================================================
// Face Information Structure
// ==============================================================================
struct FaceInfo {
    float x;                      // Normalized X coordinate [0.0, 1.0]
    float y;                      // Normalized Y coordinate [0.0, 1.0]
    float width;                  // Normalized width [0.0, 1.0]
    float height;                 // Normalized height [0.0, 1.0]
    char* faceImageData;          // Face image data (RGBA format)
    size_t faceImageLength;       // Length of face image data
    int faceImageWidth;           // Face image width in pixels
    int faceImageHeight;          // Face image height in pixels

    FaceInfo() = default;
    ~FaceInfo();

    // Disable copy operations
    FaceInfo(const FaceInfo&) = delete;
    FaceInfo& operator=(const FaceInfo&) = delete;

    // Enable move operations
    FaceInfo(FaceInfo&& other) noexcept;
    FaceInfo& operator=(FaceInfo&& other) noexcept;

    void clear();
};

// ==============================================================================
// Face Detection Result Structure
// ==============================================================================
struct FaceDetectionResult {
    char imageId[IMAGE_ID_LEN];   // Image identifier
    FaceInfo* faces;              // Array of detected faces
    int faceCount;                // Number of detected faces

    FaceDetectionResult();
    ~FaceDetectionResult();

    // Disable copy operations
    FaceDetectionResult(const FaceDetectionResult&) = delete;
    FaceDetectionResult& operator=(const FaceDetectionResult&) = delete;

    // Enable move operations
    FaceDetectionResult(FaceDetectionResult&& other) noexcept;
    FaceDetectionResult& operator=(FaceDetectionResult&& other) noexcept;

    void clear();
};

// ==============================================================================
// Picture Show Information Structure
// ==============================================================================
struct PicShowInfo {
    uint32_t picReadTime;         // Picture read time in milliseconds
    uint32_t picWidth;            // Picture width in pixels
    uint32_t picHeight;           // Picture height in pixels
    char imageId[IMAGE_ID_LEN];   // Image identifier
    char* imageRgbaData;          // Image data in RGBA format
    size_t imageRgbaLen;          // Length of RGBA data

    PicShowInfo();
    ~PicShowInfo();

    // Copy constructor and assignment (deep copy)
    PicShowInfo(const PicShowInfo& other);
    PicShowInfo& operator=(const PicShowInfo& other);

    // Move operations
    PicShowInfo(PicShowInfo&& other) noexcept;
    PicShowInfo& operator=(PicShowInfo&& other) noexcept;

    void clear();
};

// ==============================================================================
// Callback Types
// ==============================================================================
using PlayerMsgCallback = void*(*)(int handle, int iMsg, void* pData, void* pUser);

// Message types for callbacks
enum CallbackMessage {
    Callback_ShowPicId = 1,       // Show picture with ID
    Callback_Error = 2,           // Error occurred
    Callback_Complete = 3         // Operation complete
};

} // namespace ComponentInterface