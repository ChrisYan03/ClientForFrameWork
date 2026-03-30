#ifndef PICPLAYERVIDEORENDER_H
#define PICPLAYERVIDEORENDER_H

#include "PicPlayerRender.h"
#include "PicPlayerDataDef.h"
#include "imgui_internal.h"

class PicPlayerVideoRender : public PicPlayerRender
{
public:
    PicPlayerVideoRender(int cacheNum, PicShowType showType = PicShowType_Move);
    ~PicPlayerVideoRender();

    virtual void InitScene(const ImRect& rc) override;
    virtual void InitFramerate(float frame) override;

protected:
    int m_cacheNum;
    PicShowType m_showType;
};

#endif // PICPLAYERVIDEORENDER_H

