#ifndef PICPLAYERMOVIEBYSCENE_H
#define PICPLAYERMOVIEBYSCENE_H

#include "PicPlayerScene.h"

class PicPlayerMovieByScene : public PicPlayerScene
{
public:
    PicPlayerMovieByScene(const ImRect& rc, int cacheNum);
    ~PicPlayerMovieByScene();

    virtual void Advance() override;
    virtual void ClearRenderData() override;

protected:
    virtual void OnDisplayRectChanged() override;
    virtual void DrawScene() override;

protected:
    int m_cacheNum;
};

#endif // PICPLAYERMOVIEBYSCENE_H
