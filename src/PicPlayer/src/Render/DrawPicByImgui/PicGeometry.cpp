#include "PicGeometry.h"
#include "PicTexture.h"
#include "LogUtil.h"

PicGeometry::PicGeometry()
    : m_uvMin(0.0, 0.0)
    , m_uvMax(0.0, 0.0)
    , m_directionLTR(true)
    , m_curXPos(0)
    , m_picWidth(0)
    , m_picHeight(0)
    , m_uTexId(0)
{

}

PicGeometry::~PicGeometry()
{
    if (0 != m_uTexId) {
        // 归还纹理id到纹理池
        PicTexture::instance()->ReleaseTexId(m_uTexId);
    }
}

// 绘制接口
void PicGeometry::DrawImageForVideo(const ImVec2& drawStart, const ImVec2& drawEnd, double scale)
{
    auto endGap = m_picWidth * scale;
    ImVec2 posStart, posEnd;
    // 从左到右移动
    posStart = ImVec2(drawEnd.x, drawStart.y);
    posEnd = ImVec2(drawEnd.x - endGap, drawEnd.y);

    // 调用绘制单例
    PicTexture::instance()->DrawPicTexture(m_uTexId, posStart, posEnd, m_uvMin, m_uvMax);
    // 绘制人脸框
    for (size_t i = 0; i < m_faceRectVec.size(); ++i) {
        float displayedWidth = m_picWidth * (m_uvMax.x - m_uvMin.x);
        float displayedHeight = m_picHeight * (m_uvMax.y - m_uvMin.y);
        
        // Convert face rectangle coordinates from full image space to displayed image space
        // Account for the cropped region by adjusting the origin
        ImVec2 rectStart = ImVec2(
            posEnd.x + (m_faceRectVec[i].rect.Min.x - m_uvMin.x * m_picWidth) * scale, 
            posStart.y + (m_faceRectVec[i].rect.Min.y - m_uvMin.y * m_picHeight) * scale
        );
        ImVec2 rectEnd = ImVec2(
            posEnd.x + (m_faceRectVec[i].rect.Max.x - m_uvMin.x * m_picWidth) * scale, 
            posStart.y + (m_faceRectVec[i].rect.Max.y - m_uvMin.y * m_picHeight) * scale
        );
        
        PicTexture::instance()->DrawRect(rectStart, rectEnd, IM_COL32(0, 255, 0, 255));
        
        // 在人脸框上方绘制置信度标签
        char label[64];
        snprintf(label, sizeof(label), "face:%.0f%%", m_faceRectVec[i].confidence * 100); // 显示实际置信度百分比
        
        // 计算标签文本尺寸
        ImVec2 textSize = ImGui::CalcTextSize(label);
        
        // 在人脸框上方绘制标签背景
        ImVec2 labelPos = ImVec2(rectStart.x, rectStart.y - textSize.y - 4); // 4像素间距
        // 绘制标签文本
        PicTexture::instance()->DrawRecogResult(labelPos, label, IM_COL32(0, 255, 0, 255)); 
        // 在人脸框下方绘制年龄标签
        if (m_faceRectVec[i].age != -1) {
            char ageLabel[64];
            snprintf(ageLabel, sizeof(ageLabel), "age: %d", m_faceRectVec[i].age);
            
            // 计算年龄标签文本尺寸
            ImVec2 ageTextSize = ImGui::CalcTextSize(ageLabel);
            
            // 在人脸框下方绘制年龄标签
            ImVec2 ageLabelPos = ImVec2(rectStart.x, rectEnd.y + 4); // 4像素间距
            
            // 绘制年龄标签文本
            PicTexture::instance()->DrawRecogResult(ageLabelPos, ageLabel, IM_COL32(0, 255, 0, 255)); // 使用绿色显示年龄
        }
    }
}

// 绘制矩形框
void PicGeometry::DrawRectForPic(const ImVec2& drawStart, const ImVec2& drawEnd)
{
    ImRect picRect;
    picRect.Min = drawStart;
    picRect.Max = drawEnd;
    // 调用绘制单例
    PicTexture::instance()->DrawRect(picRect.Min, picRect.Max, IM_COL32(0, 0, 255, 255));
}

// 增加数据接口
void PicGeometry::AddNewPic(std::shared_ptr<PicShowInfo> data)
{
    if (data->imageRgbaData){
        m_picWidth = (int)data->picWidth;
        m_picHeight = (int)data->picHeight;
        if (0 == m_uTexId) {
            // 获取一个纹理
            m_uTexId = PicTexture::instance()->GenTexId();
        }
        // 更新纹理
        PicTexture::instance()->SetPicTexture(m_uTexId, data);
        // 获取uv
        m_uvMin.y = m_topPos / (float)data->picHeight;
        m_uvMax.y = (data->picHeight - m_bomPos) / (float)data->picHeight;
        if (m_directionLTR) {
            m_uvMin.x = 0.0f;
            m_uvMax.x = 1.0f;//(实际宽度/图片宽度)
        }
    }
}

void PicGeometry::AddFaceRecogResult(std::shared_ptr<FaceDetectionResult> data)
{
    if(data->faceCount > 0){
        for(int i = 0; i < data->faceCount; ++i){
            auto face = data->faces[i];
            ImVec2 drawStart = ImVec2(face.x * m_picWidth, face.y * m_picHeight);
            ImVec2 drawEnd = ImVec2((face.x + face.width) * m_picWidth, (face.y + face.height) * m_picHeight);
            m_faceRectVec.push_back(FaceRectWithConfidence(ImRect(drawStart, drawEnd), face.confidence, face.age));
        }
    }
}

// 设置选中回调
void PicGeometry::SetSelectionCallback(std::function<void(const std::string&)>&& func)
{
    m_selectCallback = std::move(func);
}

float PicGeometry::GetPicContentScale(float displayHeight)
{
    return displayHeight / m_picHeight;
}

