#include "PicGeometry.h"
#include "PicTexture.h"
#include "imgui_internal.h"

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
void PicGeometry::AddNewPic(const PicShowInfo &data)
{
    if (data.imageRgbaData){
        m_picWidth = (int)data.picWidth;
        m_picHeight = (int)data.picHeight;
        if (0 == m_uTexId) {
            // 获取一个纹理
            m_uTexId = PicTexture::instance()->GenTexId();
        }
        // 更新纹理
        PicTexture::instance()->SetPicTexture(m_uTexId, data);
        // 获取uv
        m_uvMin.y = m_topPos / (float)data.picHeight;
        m_uvMax.y = (data.picHeight - m_bomPos) / (float)data.picHeight;
        if (m_directionLTR) {
            m_uvMin.x = 0.0f;
            m_uvMax.x = 1.0f;//(实际宽度/图片宽度)
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
