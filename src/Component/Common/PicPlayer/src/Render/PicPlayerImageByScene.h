#ifndef PICPLAYERIMAGEBYSCENE_H
#define PICPLAYERIMAGEBYSCENE_H

#include "PicPlayerScene.h"
#include "DrawPicByImgui/PicRenderForDraw.h"
#include <memory>
#include <vector>

/**
 * @brief 静态图片场景：单图居中 / 双图左右平分。支持滚轮缩放、放大后左键拖拽平移；左上角画中画显示整图与当前视口框。
 */
class PicPlayerImageByScene : public PicPlayerScene
{
public:
    PicPlayerImageByScene(const ImRect& rc, int cacheNum);
    ~PicPlayerImageByScene() override;

    void Advance() override;
    void ClearRenderData() override;
    void UpdateRenderNodeData(std::shared_ptr<RenderNodesData> nodeData) override;

protected:
    void OnDisplayRectChanged() override;
    void DrawScene() override;

private:
    struct ViewState {
        float zoom = 1.f;
        float panX = 0.f;
        float panY = 0.f;
    };

    void TrimCacheIfNeeded();
    static ImRect SlotRect(const ImRect& full, int slotIndex, int slotCount);
    void HandleSlotInput(int slot, const ImRect& slotRect, const std::shared_ptr<PicRenderForDraw>& pic);
    void ClampPanToSlot(int slot, const ImRect& slotRect, const std::shared_ptr<PicRenderForDraw>& pic);
    void DrawImageSlot(int slot, const ImRect& slotRect, const std::shared_ptr<PicRenderForDraw>& pic);

    std::vector<std::shared_ptr<PicRenderForDraw>> m_pics;
    ViewState m_view[2];
};

#endif // PICPLAYERIMAGEBYSCENE_H
