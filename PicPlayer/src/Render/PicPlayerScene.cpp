#include "PicPlayerScene.h"

PicPlayerScene::PicPlayerScene(const ImRect& rc, int cacheNum)
    : m_displayRect(rc)
    , m_cacheNum(cacheNum)
{

}

void PicPlayerScene::SetDisplayRect(const ImRect& rect)
{
    m_displayRect = rect;
    OnDisplayRectChanged();
}

void PicPlayerScene::SetCurFramerate(float fixframe)
{
    m_fixframe = fixframe;
}

void PicPlayerScene::SceneRender()
{
    bool open = false;
    bool use_work_area = true;
    ImGuiIO& io = ImGui::GetIO();
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);
    ImGui::Begin("DrawWindow", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

    ImGui::Text("Framerate: %.1f FPS", m_fixframe);
    DrawScene();

    ImGui::End();
}
