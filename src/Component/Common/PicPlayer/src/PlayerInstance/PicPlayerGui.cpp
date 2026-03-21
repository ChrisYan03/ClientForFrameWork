#include "PicPlayerGui.h"
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#endif
#include "PicPlayerShowWindow.h"
#include "PicPlayerRender.h"

PicPlayerGui::PicPlayerGui()
    : m_renderFactory(nullptr)
{

}

PicPlayerGui::~PicPlayerGui()
{

}


std::shared_ptr<PicPlayerGui> PicPlayerGui::Create(Window_ShowID wid)
{   
    return std::make_shared<PicPlayerShowWindow>(wid);
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
            render->PlayRender();
        }
    }
}

PicPlayerRender* PicPlayerGui::GetRender() const
{
    if (m_renderFactory) {
        PicPlayerRender* render = m_renderFactory->GetRender();
        return render;
    }
    return nullptr;
}

