#include "PicPlayerImageByScene.h"
#include "../NodeDataDef/NodesDataForDraw.h"
#include "DrawPicByImgui/PicGeometry.h"
#include "imgui.h"
#include <algorithm>
#include <cstdio>

namespace {
constexpr float kZoomMin = 1.f;
constexpr float kZoomMax = 12.f;
constexpr float kWheelZoomStep = 1.12f;
constexpr float kPipWidth = 210.f;
constexpr float kPipHeight = 150.f;
constexpr float kPipMargin = 12.f;
}

PicPlayerImageByScene::PicPlayerImageByScene(const ImRect& rc, int cacheNum)
    : PicPlayerScene(rc, cacheNum)
{
}

PicPlayerImageByScene::~PicPlayerImageByScene() = default;

void PicPlayerImageByScene::Advance()
{
}

void PicPlayerImageByScene::ClearRenderData()
{
    m_pics.clear();
    m_view[0] = ViewState{};
    m_view[1] = ViewState{};
}

void PicPlayerImageByScene::UpdateRenderNodeData(std::shared_ptr<RenderNodesData> nodeData)
{
    if (!nodeData || nodeData->GetComDataList().empty())
        return;

    auto& comList = nodeData->GetComDataList();
    for (auto iterCmd = comList.begin(); iterCmd != comList.end(); ++iterCmd) {
        if ((*iterCmd)->RenderType() == (int)NodesType::PicDataType) {
            auto* picDataPtr = static_cast<PicData*>(iterCmd->get());
            auto it = std::find_if(m_pics.begin(), m_pics.end(),
                [picDataPtr](const std::shared_ptr<PicRenderForDraw>& p) {
                    return picDataPtr->picShowData->imageId == p->GetPicId();
                });
            if (it == m_pics.end()) {
                auto cur = std::make_shared<PicRenderForDraw>(picDataPtr->picShowData->imageId);
                cur->SetPicInfo(picDataPtr->picShowData);
                m_pics.push_back(cur);
            }
        } else if ((*iterCmd)->RenderType() == (int)NodesType::FaceRecogType) {
            auto* recogDataPtr = static_cast<FaceRecogData*>(iterCmd->get());
            auto it = std::find_if(m_pics.begin(), m_pics.end(),
                [recogDataPtr](const std::shared_ptr<PicRenderForDraw>& p) {
                    return recogDataPtr->picDetectionResult->imageId == p->GetPicId();
                });
            if (it != m_pics.end())
                (*it)->SetFaceRecogResult(recogDataPtr->picDetectionResult);
        }
    }
    TrimCacheIfNeeded();
}

void PicPlayerImageByScene::OnDisplayRectChanged()
{
    m_view[0] = ViewState{};
    m_view[1] = ViewState{};
}

ImRect PicPlayerImageByScene::SlotRect(const ImRect& full, int slotIndex, int slotCount)
{
    if (slotCount <= 1)
        return full;
    const float midX = (full.Min.x + full.Max.x) * 0.5f;
    if (slotIndex == 0)
        return ImRect(full.Min.x, full.Min.y, midX, full.Max.y);
    return ImRect(midX, full.Min.y, full.Max.x, full.Max.y);
}

void PicPlayerImageByScene::TrimCacheIfNeeded()
{
    while (static_cast<int>(m_pics.size()) > m_cacheNum) {
        SyncRemovePic(m_pics.front()->GetPicId());
        m_pics.erase(m_pics.begin());
    }
}

void PicPlayerImageByScene::HandleSlotInput(int slot, const ImRect& slotRect,
    const std::shared_ptr<PicRenderForDraw>& pic)
{
    if (!pic)
        return;
    auto geo = pic->GetPicGeoPtr();
    if (!geo || geo->GetContentWidthPx() <= 1.f || geo->GetContentHeightPx() <= 1.f)
        return;

    char btnId[40];
    snprintf(btnId, sizeof(btnId), "##imgStaticSlot%d", slot);
    ImGui::SetCursorScreenPos(slotRect.Min);
    ImGui::InvisibleButton(btnId, ImVec2(slotRect.GetWidth(), slotRect.GetHeight()));

    if (!ImGui::IsItemHovered())
        return;

    ImGuiIO& io = ImGui::GetIO();
    if (io.MouseWheel != 0.f) {
        float z = m_view[slot].zoom;
        if (io.MouseWheel > 0.f)
            z *= kWheelZoomStep;
        else
            z /= kWheelZoomStep;
        z = std::max(kZoomMin, std::min(kZoomMax, z));
        if (z <= kZoomMin + 1e-3f) {
            m_view[slot].zoom = kZoomMin;
            m_view[slot].panX = 0.f;
            m_view[slot].panY = 0.f;
        } else {
            m_view[slot].zoom = z;
        }
    }

    if (m_view[slot].zoom > kZoomMin + 1e-3f && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        m_view[slot].panX += io.MouseDelta.x;
        m_view[slot].panY += io.MouseDelta.y;
    }

    ClampPanToSlot(slot, slotRect, pic);
}

void PicPlayerImageByScene::ClampPanToSlot(int slot, const ImRect& slotRect,
    const std::shared_ptr<PicRenderForDraw>& pic)
{
    auto geo = pic->GetPicGeoPtr();
    const float cw = geo->GetContentWidthPx();
    const float ch = geo->GetContentHeightPx();
    const float W = slotRect.GetWidth();
    const float H = slotRect.GetHeight();
    const float s0 = std::min(W / cw, H / ch);
    const float s = s0 * m_view[slot].zoom;
    const float drawW = cw * s;
    const float drawH = ch * s;
    const float cx = (slotRect.Min.x + slotRect.Max.x) * 0.5f;
    const float cy = (slotRect.Min.y + slotRect.Max.y) * 0.5f;
    float left = cx - drawW * 0.5f + m_view[slot].panX;
    float top = cy - drawH * 0.5f + m_view[slot].panY;

    if (drawW > W) {
        const float minLeft = slotRect.Max.x - drawW;
        const float maxLeft = slotRect.Min.x;
        left = std::max(minLeft, std::min(maxLeft, left));
    } else {
        left = cx - drawW * 0.5f;
        m_view[slot].panX = 0.f;
    }

    if (drawH > H) {
        const float minTop = slotRect.Max.y - drawH;
        const float maxTop = slotRect.Min.y;
        top = std::max(minTop, std::min(maxTop, top));
    } else {
        top = cy - drawH * 0.5f;
        m_view[slot].panY = 0.f;
    }

    m_view[slot].panX = left - (cx - drawW * 0.5f);
    m_view[slot].panY = top - (cy - drawH * 0.5f);
}

void PicPlayerImageByScene::DrawImageSlot(int slot, const ImRect& slotRect,
    const std::shared_ptr<PicRenderForDraw>& pic)
{
    if (!pic)
        return;
    auto geo = pic->GetPicGeoPtr();
    const float cw = geo->GetContentWidthPx();
    const float ch = geo->GetContentHeightPx();
    if (cw <= 1.f || ch <= 1.f)
        return;

    const ImVec2 u0 = geo->GetUvMin();
    const ImVec2 u1 = geo->GetUvMax();

    const float W = slotRect.GetWidth();
    const float H = slotRect.GetHeight();
    if (W <= 1.f || H <= 1.f)
        return;

    const float s0 = std::min(W / cw, H / ch);
    const float s = s0 * m_view[slot].zoom;
    const float drawW = cw * s;
    const float drawH = ch * s;
    const float cx = (slotRect.Min.x + slotRect.Max.x) * 0.5f;
    const float cy = (slotRect.Min.y + slotRect.Max.y) * 0.5f;
    const float left = cx - drawW * 0.5f + m_view[slot].panX;
    const float top = cy - drawH * 0.5f + m_view[slot].panY;
    const ImRect dst(left, top, left + drawW, top + drawH);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->PushClipRect(slotRect.Min, slotRect.Max, true);
    geo->DrawImageToRectUv(dst, u0, u1);
    dl->PopClipRect();

    const bool showPip = m_view[slot].zoom > kZoomMin + 1e-3f;
    if (!showPip)
        return;

    const ImRect pipOuter(
        ImVec2(slotRect.Min.x + kPipMargin, slotRect.Min.y + kPipMargin),
        ImVec2(slotRect.Min.x + kPipMargin + kPipWidth, slotRect.Min.y + kPipMargin + kPipHeight));
    const float ts = std::min(kPipWidth / cw, kPipHeight / ch);
    const float tw = cw * ts;
    const float th = ch * ts;
    const float tcx = (pipOuter.Min.x + pipOuter.Max.x) * 0.5f;
    const float tcy = (pipOuter.Min.y + pipOuter.Max.y) * 0.5f;
    const ImRect thumb(tcx - tw * 0.5f, tcy - th * 0.5f, tcx + tw * 0.5f, tcy + th * 0.5f);

    dl->AddRectFilled(pipOuter.Min, pipOuter.Max, IM_COL32(8, 8, 12, 200));
    dl->PushClipRect(pipOuter.Min, pipOuter.Max, true);
    geo->DrawImageToRectUv(thumb, u0, u1);

    const float sx0 = std::max(slotRect.Min.x, dst.Min.x);
    const float sx1 = std::min(slotRect.Max.x, dst.Max.x);
    const float sy0 = std::max(slotRect.Min.y, dst.Min.y);
    const float sy1 = std::min(slotRect.Max.y, dst.Max.y);
    if (sx1 > sx0 && sy1 > sy0 && drawW > 1e-3f && drawH > 1e-3f) {
        const float visU0 = u0.x + (u1.x - u0.x) * (sx0 - dst.Min.x) / drawW;
        const float visU1 = u0.x + (u1.x - u0.x) * (sx1 - dst.Min.x) / drawW;
        const float visV0 = u0.y + (u1.y - u0.y) * (sy0 - dst.Min.y) / drawH;
        const float visV1 = u0.y + (u1.y - u0.y) * (sy1 - dst.Min.y) / drawH;

        auto mapU = [&](float u) {
            return thumb.Min.x + (u - u0.x) / (u1.x - u0.x) * tw;
        };
        auto mapV = [&](float v) {
            return thumb.Min.y + (v - u0.y) / (u1.y - u0.y) * th;
        };
        const ImVec2 r0(mapU(visU0), mapV(visV0));
        const ImVec2 r1(mapU(visU1), mapV(visV1));
        dl->AddRect(r0, r1, IM_COL32(255, 220, 60, 255), 0.f, 0, 2.5f);
    }
    dl->PopClipRect();
    dl->AddRect(pipOuter.Min, pipOuter.Max, IM_COL32(200, 200, 210, 220), 0.f, 0, 1.5f);
}

void PicPlayerImageByScene::DrawScene()
{
    if (m_pics.empty())
        return;

    const int n = static_cast<int>(m_pics.size());
    const int slotCount = (n >= 2) ? 2 : 1;

    std::shared_ptr<PicRenderForDraw> slotPic[2];
    if (slotCount == 2) {
        slotPic[0] = m_pics[static_cast<size_t>(n - 2)];
        slotPic[1] = m_pics[static_cast<size_t>(n - 1)];
    } else {
        slotPic[0] = m_pics[static_cast<size_t>(n - 1)];
        slotPic[1] = nullptr;
    }

    for (int s = 0; s < slotCount; ++s) {
        const ImRect sr = SlotRect(m_displayRect, s, slotCount);
        HandleSlotInput(s, sr, slotPic[s]);
        DrawImageSlot(s, sr, slotPic[s]);
    }
}
