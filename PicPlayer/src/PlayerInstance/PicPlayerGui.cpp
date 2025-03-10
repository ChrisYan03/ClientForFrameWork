#include "PicPlayerGui.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glfw3.h"
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw3native.h"
#endif
#include "PicPlayerShowWindow.h"

PicPlayerGui::PicPlayerGui()
{

}

PicPlayerGui::~PicPlayerGui()
{

}


PicPlayerGui* PicPlayerGui::Create(Window_ShowID wid)
{
    PicPlayerShowWindow* windowGui = new PicPlayerShowWindow(wid);
    return windowGui;
}

void PicPlayerGui::SetIRenderFactory(IRenderFactory* renderFactory)
{
    m_renderFactory = renderFactory;
}

void PicPlayerGui::RenderScene()
{
    if (m_renderFactory) {
        PicPlayerRender* render = m_renderFactory->GetRender();
        if (render){
            //render->Render();
        }
    }
}

PicPlayerRender* PicPlayerGui::GetRender()
{
    if (m_renderFactory) {
        PicPlayerRender* render = m_renderFactory->GetRender();
        return render;
    }
    return nullptr;
}
