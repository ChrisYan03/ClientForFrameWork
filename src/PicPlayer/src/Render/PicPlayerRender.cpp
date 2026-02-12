#include "PicPlayerRender.h"
#include "imgui_impl_opengl3_loader.h"

PicPlayerRender::PicPlayerRender()
    : m_renderSync(std::make_shared<PicPlayerRenderSync>())
{

}

PicPlayerRender::~PicPlayerRender()
{

}

void PicPlayerRender::PlayRender()
{
    //同步数据
    RenderNodesData* pData = m_renderSync->BeginSync();
    if (pData) {
        UpdateRenderNodesData(pData);
        m_renderSync->EndSync(pData);
    }

    if (m_playScene) {
        m_playScene->Advance();
        m_playScene->SceneRender();
    }
}

void PicPlayerRender::ClearRenderCache()
{
    if (m_playScene) {
        m_playScene->ClearRenderData();
    }
}

void PicPlayerRender::UpdateRenderNodesData(RenderNodesData* data)
{
    if (m_playScene) {
        std::shared_ptr<RenderNodesData> sharedData(data, [](RenderNodesData* ptr) { delete ptr; });
        m_playScene->UpdateRenderNodeData(sharedData);
    }
}

std::shared_ptr<PicPlayerRenderSync> PicPlayerRender::GetSynchronizer() const
{
    return m_renderSync;
}

void PicPlayerRender::UpdateViewport(int width, int height)
{
    if (m_playScene) {
        m_playScene->SetDisplayRect(ImRect(4, 6, width - 4, height - 6));
    }
}

PicPlayerScene* PicPlayerRender::GetScene() const
{
    return m_playScene.get();
}

