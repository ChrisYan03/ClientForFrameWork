#include "ImageViewPlayerHostPicLoader.h"
#include "StbImage/stb_image.h"
#include "LogUtil.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <cstdlib>
#include <cstring>
#include <cstdint>

namespace {
static uint16_t readU16(const unsigned char* p, bool le)
{
    return le ? static_cast<uint16_t>(p[0] | (p[1] << 8))
              : static_cast<uint16_t>((p[0] << 8) | p[1]);
}

static char* rotateRgbaByExif(const unsigned char* src, int srcW, int srcH, int orientation, int& outW, int& outH)
{
    outW = srcW;
    outH = srcH;
    if (!src || srcW <= 0 || srcH <= 0)
        return nullptr;
    if (!(orientation == 3 || orientation == 6 || orientation == 8))
        return nullptr;

    if (orientation == 6 || orientation == 8) {
        outW = srcH;
        outH = srcW;
    }

    const size_t len = static_cast<size_t>(outW) * static_cast<size_t>(outH) * 4u;
    auto* dst = static_cast<unsigned char*>(std::malloc(len));
    if (!dst)
        return nullptr;

    auto writePixel = [&](int dx, int dy, const unsigned char* p) {
        const size_t di = (static_cast<size_t>(dy) * static_cast<size_t>(outW) + static_cast<size_t>(dx)) * 4u;
        dst[di + 0] = p[0];
        dst[di + 1] = p[1];
        dst[di + 2] = p[2];
        dst[di + 3] = p[3];
    };

    for (int sy = 0; sy < srcH; ++sy) {
        for (int sx = 0; sx < srcW; ++sx) {
            const size_t si = (static_cast<size_t>(sy) * static_cast<size_t>(srcW) + static_cast<size_t>(sx)) * 4u;
            const unsigned char* p = src + si;
            int dx = sx;
            int dy = sy;
            if (orientation == 3) { // rotate 180
                dx = srcW - 1 - sx;
                dy = srcH - 1 - sy;
            } else if (orientation == 6) { // rotate 90 CW
                dx = srcH - 1 - sy;
                dy = sx;
            } else if (orientation == 8) { // rotate 90 CCW
                dx = sy;
                dy = srcW - 1 - sx;
            }
            writePixel(dx, dy, p);
        }
    }
    return reinterpret_cast<char*>(dst);
}

// 从 JPEG EXIF 中提取 Orientation(1..8)，失败返回 0
static int parseJpegExifOrientation(const QByteArray& bytes)
{
    const auto* b = reinterpret_cast<const unsigned char*>(bytes.constData());
    const int n = bytes.size();
    if (n < 4 || b[0] != 0xFF || b[1] != 0xD8)
        return 0;

    int i = 2;
    while (i + 4 <= n) {
        if (b[i] != 0xFF)
            break;
        const unsigned char marker = b[i + 1];
        i += 2;
        if (marker == 0xDA || marker == 0xD9)
            break;
        if (i + 2 > n)
            break;
        const int segLen = (b[i] << 8) | b[i + 1];
        if (segLen < 2 || i + segLen > n)
            break;
        if (marker == 0xE1 && segLen >= 8) {
            const unsigned char* exif = b + i + 2;
            const int exifLen = segLen - 2;
            if (exifLen >= 14 && std::memcmp(exif, "Exif\0\0", 6) == 0) {
                const unsigned char* tiff = exif + 6;
                const bool le = (tiff[0] == 'I' && tiff[1] == 'I');
                const bool be = (tiff[0] == 'M' && tiff[1] == 'M');
                if (!le && !be)
                    return 0;
                const uint32_t ifd0Off = le
                    ? static_cast<uint32_t>(tiff[4] | (tiff[5] << 8) | (tiff[6] << 16) | (tiff[7] << 24))
                    : static_cast<uint32_t>((tiff[4] << 24) | (tiff[5] << 16) | (tiff[6] << 8) | tiff[7]);
                if (ifd0Off + 2 > static_cast<uint32_t>(exifLen - 6))
                    return 0;
                const unsigned char* ifd = tiff + ifd0Off;
                const uint16_t count = readU16(ifd, le);
                for (uint16_t k = 0; k < count; ++k) {
                    const unsigned char* ent = ifd + 2 + k * 12;
                    if (ent + 12 > exif + exifLen)
                        break;
                    const uint16_t tag = readU16(ent, le);
                    if (tag == 0x0112) {
                        const uint16_t val = readU16(ent + 8, le);
                        return (val >= 1 && val <= 8) ? static_cast<int>(val) : 0;
                    }
                }
            }
        }
        i += segLen;
    }
    return 0;
}
}

bool ImageViewLoadImageFileToPicShowInfo(const QString& absolutePath, PicShowInfo& out, QString* errorOut)
{
    out.clear();

    QFile file(absolutePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorOut)
            *errorOut = QStringLiteral("open failed");
        LOG_ERROR("ImageViewLoadImageFileToPicShowInfo: cannot open {}", absolutePath.toStdString());
        return false;
    }

    const QByteArray bytes = file.readAll();
    if (bytes.isEmpty()) {
        if (errorOut)
            *errorOut = QStringLiteral("empty file");
        return false;
    }

    int w = 0;
    int h = 0;
    int ch = 0;
    unsigned char* pixels = stbi_load_from_memory(
        reinterpret_cast<const stbi_uc*>(bytes.constData()),
        static_cast<int>(bytes.size()),
        &w,
        &h,
        &ch,
        4);
    if (!pixels || w <= 0 || h <= 0) {
        if (errorOut)
            *errorOut = QString::fromUtf8(stbi_failure_reason());
        LOG_ERROR("ImageViewLoadImageFileToPicShowInfo: stbi_load_from_memory failed for {}",
            absolutePath.toStdString());
        if (pixels)
            stbi_image_free(pixels);
        return false;
    }

    const int exifOrientation = parseJpegExifOrientation(bytes);

    int finalW = w;
    int finalH = h;
    char* buf = rotateRgbaByExif(pixels, w, h, exifOrientation, finalW, finalH);

    const size_t len = static_cast<size_t>(finalW) * static_cast<size_t>(finalH) * 4u;
    if (!buf)
        buf = static_cast<char*>(std::malloc(len));
    if (!buf) {
        stbi_image_free(pixels);
        if (errorOut)
            *errorOut = QStringLiteral("malloc failed");
        return false;
    }
    if (finalW == w && finalH == h)
        std::memcpy(buf, pixels, len);
    stbi_image_free(pixels);

    out.picReadTime = 0;
    out.picWidth = static_cast<uint32_t>(finalW);
    out.picHeight = static_cast<uint32_t>(finalH);
    out.imageRgbaData = buf;
    out.imageRgbaLen = len;
    std::memset(out.imageId, 0, IMAGE_ID_LEN);
    const QByteArray baseName = QFileInfo(absolutePath).fileName().toUtf8();
    std::strncpy(out.imageId, baseName.constData(), IMAGE_ID_LEN - 1);

    LOG_INFO("ImageViewLoadImageFileToPicShowInfo: OK {} -> {}x{} id='{}'",
        absolutePath.toStdString(),
        finalW,
        finalH,
        out.imageId);
    return true;
}
