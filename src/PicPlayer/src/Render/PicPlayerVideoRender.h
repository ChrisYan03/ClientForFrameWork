#ifndef PICPLAYERVIDEORENDER_H
#define PICPLAYERVIDEORENDER_H

#include "PicPlayerRender.h"
#include "imgui_internal.h"

class PicPlayerVideoRender : public PicPlayerRender
{
public:
    PicPlayerVideoRender(int cacheNum);
    ~PicPlayerVideoRender();

    virtual void InitScene(const ImRect& rc) override;
    virtual void InitFramerate(float frame) override;

protected:
    int m_cacheNum;
};

#endif // PICPLAYERVIDEORENDER_H

