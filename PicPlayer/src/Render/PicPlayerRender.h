#ifndef PICPLAYERRENDER_H
#define PICPLAYERRENDER_H

#include "PicPlayerScene.h"
#include "PicPlayerRenderSync.h"
#include "../NodeDataDef/NodesData.h"
#include "imgui_internal.h"

class PicPlayerRender
{
public:
    PicPlayerRender();
    ~PicPlayerRender();

    virtual void InitScene(const ImRect& rc) = 0;
    virtual void InitFramerate(float frame) = 0;
    virtual void PlayRender();
    virtual void ClearRenderCache();
    virtual void UpdateRenderNodesData(RenderNodesData* data);
    std::shared_ptr<PicPlayerRenderSync> GetSynchronizer() const;
    void UpdateViewport(int width, int height);
    PicPlayerScene* GetScene() const;

protected:
    std::unique_ptr<PicPlayerScene> m_playScene;
    std::shared_ptr<PicPlayerRenderSync> m_renderSync;
};

#endif // PICPLAYERRENDER_H
