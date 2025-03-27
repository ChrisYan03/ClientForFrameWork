#ifndef PICPLAYERGUI_H
#define PICPLAYERGUI_H

#include "../PicPlayerDataDef.h"
#include <memory>

class PicPlayerRender;
class PicPlayerGui
{
public:
    class IRenderFactory{
        friend class PicPlayerGui;
    protected:
        virtual PicPlayerRender* GetRender() const = 0;
    };

public:
    PicPlayerGui();
    virtual ~PicPlayerGui();

    static std::shared_ptr<PicPlayerGui> Create(Window_ShowID wid);
    virtual void Destroy() = 0;
    virtual int RunRendLoop() = 0;
    virtual void Quit() = 0;

    void SetIRenderFactory(IRenderFactory* renderFactory);
    void RenderScene();

protected:
    PicPlayerRender* GetRender() const ;

protected:
    IRenderFactory* m_renderFactory;
};

#endif // PICPLAYERGUI_H
