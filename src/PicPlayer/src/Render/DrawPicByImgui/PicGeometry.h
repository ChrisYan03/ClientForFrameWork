#ifndef PICGEOMETRY_H
#define PICGEOMETRY_H

#include "../PicPlayerDataDef.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <string>
#include <functional>
#include <memory>
#include <vector>

struct FaceRectWithConfidence {
    ImRect rect;
    float confidence;
    
    FaceRectWithConfidence(const ImRect& r, float conf) 
        : rect(r), confidence(conf) {}
};

class PicGeometry
{
public:
    PicGeometry();
    virtual ~PicGeometry();

    // 绘制图片接口
    void DrawImageForVideo(const ImVec2& drawStart, const ImVec2& drawEnd, double scale);
    // 绘制矩形框
    void DrawRectForPic(const ImVec2& drawStart, const ImVec2& drawEnd);
    // 增加数据接口
    void AddNewPic(std::shared_ptr<PicShowInfo> data);
    // 增加人脸结果接口
    void AddFaceRecogResult(std::shared_ptr<FaceDetectionResult> data);
    // 设置选中回调
    void SetSelectionCallback(std::function<void(const std::string&)>&& func);
    // 获取图片的宽
    int GetPicWidth() const { return static_cast<int>(m_picWidth * (m_uvMax.x - m_uvMin.x));  }
    // 获取图片的高
    int GetPicHeight() const { return m_picHeight; }

    float GetPicContentScale(float displayHeight);

private:
    ImVec2 m_uvMin;
    ImVec2 m_uvMax;
    // 用于记录X的更新位置
    const int m_topPos = 20;
    const int m_bomPos = 20;
    bool m_directionLTR;
    int m_curXPos;
    // 用于记录图片的宽和高
    int m_picWidth;
    int m_picHeight;
    // 纹理id
    uint32_t m_uTexId;
    // 选中回调
    std::function<void(const std::string&)> m_selectCallback;
    std::vector<FaceRectWithConfidence> m_faceRectVec;
};

#endif // PICGEOMETRY_H

