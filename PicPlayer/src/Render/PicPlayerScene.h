#ifndef PICPLAYERSENCE_H
#define PICPLAYERSENCE_H

#include "../NodeDataDef/NodesData.h"
#include "PicPlayerRenderSync.h"
#include "imgui_internal.h"

class PicPlayerScene
{
public:
    PicPlayerScene(const ImRect& rc, int cacheNum);
    virtual ~PicPlayerScene() = default;

    void SetRenderSync(std::shared_ptr<PicPlayerRenderSync> renderSync);

    virtual void Advance() = 0;
    virtual void ClearRenderData() = 0;
    virtual void UpdateRenderNodeData(std::shared_ptr<RenderNodesData> nodeData) = 0;

    virtual void SceneRender();
    void SetCurFramerate(float fixframe);
    void SetDisplayRect(const ImRect& rect);
    void SyncRemovePic(const std::string& picId);

protected:
    virtual void OnDisplayRectChanged() = 0;
    virtual void DrawScene() = 0;
    void DoScale();

protected:
    ImRect m_displayRect;     // 可视范围
    float  m_fixframe;
    int m_cacheNum;
    const int m_fixMoveSpeed = 4;
    bool m_directionLTR;
    std::shared_ptr<PicPlayerRenderSync> m_pRenderSync;
};

#endif // PICPLAYERSENCE_H
