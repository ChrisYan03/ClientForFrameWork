#ifndef PICPLAYERSENCE_H
#define PICPLAYERSENCE_H

#include "../PicPlayerDataDef.h"
#include <queue>
#include "imgui_internal.h"

class PicPlayerScene
{
public:
    PicPlayerScene(const ImRect& rc, int cacheNum);
    virtual ~PicPlayerScene() = default;

    virtual void Advance() = 0;
    virtual void ClearRenderData() = 0;
    virtual void SceneRender();

    void SetCurFramerate(float fixframe);
    void SetDisplayRect(const ImRect& rect);

protected:
    virtual void OnDisplayRectChanged() = 0;
    virtual void DrawScene() = 0;

protected:
    ImRect m_displayRect;     // 可视范围
    float  m_fixframe;

    int m_cacheNum;
};

#endif // PICPLAYERSENCE_H
