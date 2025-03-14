#ifndef PICPLAYERGUI_H
#define PICPLAYERGUI_H

#include "../PicPlayerDataDef.h"

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
    ~PicPlayerGui();

    static PicPlayerGui* Create(Window_ShowID wid);
    virtual void Destroy() {};
    virtual int RunRendLoop() {};
    virtual void Quit() {};

    void SetIRenderFactory(IRenderFactory* renderFactory);
    void RenderScene();
    PicPlayerRender* GetRender();

protected:
    IRenderFactory* m_renderFactory;
};

#endif // PICPLAYERGUI_H
