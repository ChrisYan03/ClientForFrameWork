#ifndef IMAGEVIEWPLAYERHOSTPICLOADER_H
#define IMAGEVIEWPLAYERHOSTPICLOADER_H

#include "PicPlayerDataDef.h"
#include <QString>

/// 从绝对路径读取 JPG 等（stb_image），填充 PicShowInfo（RGBA，imageRgbaData 需 malloc）
bool ImageViewLoadImageFileToPicShowInfo(const QString& absolutePath, PicShowInfo& out, QString* errorOut = nullptr);

#endif
