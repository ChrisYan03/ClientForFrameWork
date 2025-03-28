#ifndef PICPLAYERMOVIEBYSCENE_H
#define PICPLAYERMOVIEBYSCENE_H

#include "PicPlayerScene.h"
#include "DrawPicByImgui/PicRenderForDraw.h"

class PicPlayerMovieByScene : public PicPlayerScene
{
public:
    PicPlayerMovieByScene(const ImRect& rc, int cacheNum);
    ~PicPlayerMovieByScene();

    virtual void Advance() override;
    virtual void ClearRenderData() override;
    virtual void UpdateRenderNodeData(std::shared_ptr<RenderNodesData> nodeData) override;

protected:
    virtual void OnDisplayRectChanged() override;
    virtual void DrawScene() override;
    void MoveStep();
    void CheckDrawCache();
    std::shared_ptr<PicRenderForDraw> GetPicDrawPtr(int index) const;

private:
    bool CheckRunSafety();
    int CalculateRemainLen(const std::vector<std::shared_ptr<PicRenderForDraw>>& picVec);
    void SetGeometryCallback(std::shared_ptr<PicRenderForDraw> picDrawPtr);
    void SetPicInfoToComponent(int index);

protected:
    int m_curIndex;
    // 移动相关
    int m_moveSpeed;
    int m_picMovePos;
    std::string m_curShowid;
    std::list<std::shared_ptr<PicRenderForDraw>> m_picList;
};

#endif // PICPLAYERMOVIEBYSCENE_H
