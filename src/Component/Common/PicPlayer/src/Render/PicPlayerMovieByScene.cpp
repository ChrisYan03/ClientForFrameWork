#include "PicPlayerMovieByScene.h"
#include "../NodeDataDef/NodesDataForDraw.h"
#include "PicPlayerLog.h"

PicPlayerMovieByScene::PicPlayerMovieByScene(const ImRect& rc, int cacheNum)
    : PicPlayerScene(rc, cacheNum)
    , m_curIndex(0)
    , m_moveSpeed(2)
    , m_picMovePos(0)
    , m_curShowid("")
{

}

PicPlayerMovieByScene::~PicPlayerMovieByScene()
{

}

void PicPlayerMovieByScene::Advance()
{
    if (!CheckRunSafety())
        return;

    auto curIndex = m_curIndex;

    DoScale();
    // todo:记录拖动
    // DragingRecord();
    // 移动
    MoveStep();
    // 仅当当前图片已完全显示时才触发
    SetPicInfoToComponent(curIndex);
}

void PicPlayerMovieByScene::ClearRenderData()
{
    m_picList.clear();
}

void PicPlayerMovieByScene::UpdateRenderNodeData(std::shared_ptr<RenderNodesData> nodeData)
{
    if (nodeData->GetComDataList().empty())
        return;
    auto& comdataList = nodeData->GetComDataList();
    auto iterCmd = comdataList.begin();
    while(iterCmd != comdataList.end()) {
        if (iterCmd->get()->RenderType() == (int)NodesType::PicDataType) {
            auto picDataPtr = static_cast<PicData*>(iterCmd->get());
            auto iterPtr = std::find_if(m_picList.begin(), m_picList.end(), [picDataPtr](std::shared_ptr<PicRenderForDraw> picPtr) {
                return picDataPtr->picShowData->imageId == picPtr->GetPicId();
            });
            if (iterPtr == m_picList.end()) {
                auto curPtr = std::make_shared<PicRenderForDraw>(picDataPtr->picShowData->imageId);
                curPtr->SetPicInfo(picDataPtr->picShowData);
                m_picList.emplace_back(curPtr);
            }
        }
        else if (iterCmd->get()->RenderType() == (int)NodesType::FaceRecogType) {
            auto recogDataPtr = static_cast<FaceRecogData*>(iterCmd->get());
            auto iterPtr = std::find_if(m_picList.begin(), m_picList.end(), [recogDataPtr](std::shared_ptr<PicRenderForDraw> picPtr) {
                return recogDataPtr->picDetectionResult->imageId == picPtr->GetPicId();
            });
            if (iterPtr != m_picList.end()) {
                (*iterPtr)->SetFaceRecogResult(recogDataPtr->picDetectionResult);
            }
        }
        iterCmd++;
    }
}

void PicPlayerMovieByScene::OnDisplayRectChanged()
{

}

void PicPlayerMovieByScene::DrawScene()
{
    if (!CheckRunSafety())
        return;
    std::vector<std::shared_ptr<PicRenderForDraw>> picVec;
    for (auto iterPic = m_picList.begin(); iterPic != m_picList.end(); ++iterPic) {
        picVec.push_back(*iterPic);
    }

    auto curPic = picVec[m_curIndex];
    float scale = curPic->GetShowScale();
    double displayWidth = (double)curPic->GetPicWidth();
    double displayPos = m_picMovePos * scale;

    double nodePostion = 0.0;
    if (m_directionLTR) {
        nodePostion = m_displayRect.Min.x - (displayWidth - displayPos);
        auto curIndex = m_curIndex;
        while (curIndex >= 0) {
            curPic = picVec[curIndex];
            displayWidth = (double)curPic->GetPicWidth();
            ImVec2 moveStart(nodePostion, m_displayRect.Min.y + 20);
            ImVec2 moveEnd(moveStart.x + displayWidth, moveStart.y + curPic->GetPicContentHeight());
            curPic->GetPicGeoPtr()->DrawImageForVideo(moveStart, moveEnd, curPic->GetShowScale());
            nodePostion += displayWidth;
            --curIndex;
        }
    }
    else {
        nodePostion = m_displayRect.Max.x - displayPos;
        auto curIndex = m_curIndex;
        while (curIndex >= 0) {
            curPic = picVec[curIndex];
            displayWidth = (double)curPic->GetPicWidth();
            if (curIndex != m_curIndex)
                nodePostion -= displayWidth;
            ImVec2 moveStart(nodePostion, m_displayRect.Min.y  + 20);
            ImVec2 moveEnd(moveStart.x + displayWidth, moveStart.y + curPic->GetPicContentHeight());
            curPic->GetPicGeoPtr()->DrawImageForVideo(moveStart, moveEnd, curPic->GetShowScale());
            --curIndex;
        }
    }
}

void PicPlayerMovieByScene::MoveStep()
{
    if (!CheckRunSafety())
        return;

    std::vector<std::shared_ptr<PicRenderForDraw>> picVec;
    for (auto iterPic = m_picList.begin(); iterPic != m_picList.end(); ++iterPic) {
        iterPic->get()->SetPicShowScale(m_displayRect.GetHeight() - 40);
        picVec.push_back(*iterPic);
    }

    auto curPic = picVec[m_curIndex];
    float scale = curPic->GetShowScale();
    // 滚动模式
    {
        int remainLen = CalculateRemainLen(picVec);
        auto displayPos = m_picMovePos * scale;
        remainLen -= displayPos;
        // 根据帧率动态调整移动速度
        // 基准：60Hz时速度为4，保持视觉一致性
        int dynamicSpeed = static_cast<int>(m_fixMoveSpeed * 60.0f / m_fixframe);
        // 限制速度范围，避免过高或过低
        dynamicSpeed = std::max(1, std::min(dynamicSpeed, 4));
        m_moveSpeed = std::min<int>(remainLen, dynamicSpeed);
        displayPos += m_moveSpeed;

        auto displayWidth = curPic->GetPicWidth();
        while (ceil(displayPos) >= displayWidth && m_curIndex < picVec.size() - 1) {
            displayPos -= displayWidth;
            ++m_curIndex;
            displayWidth = picVec[m_curIndex]->GetPicWidth();
        }
        m_picMovePos = displayPos/scale;
    }
}

void PicPlayerMovieByScene::CheckDrawCache()
{
    while(m_picList.size() >= m_cacheNum && m_curIndex > 0) {
        auto iter = m_picList.begin();
        if (iter != m_picList.end()) {
            SyncRemovePic((*iter)->GetPicId());
            m_picList.pop_front();
            --m_curIndex;
        }
    }
}

std::shared_ptr<PicRenderForDraw> PicPlayerMovieByScene::GetPicDrawPtr(int index) const
{
    if (index < 0 || index >= m_picList.size())
        return nullptr;
    auto iter = m_picList.begin();
    std::advance(iter, index);
    if (iter == m_picList.end())
        return nullptr;

    return *iter;
}

bool PicPlayerMovieByScene::CheckRunSafety()
{
    if (m_picList.empty())
        return false;
    if (m_curIndex < 0 || m_curIndex >= m_picList.size())
        return false;
    return true;
}

int PicPlayerMovieByScene::CalculateRemainLen(const std::vector<std::shared_ptr<PicRenderForDraw>>& picVec)
{
    int remainLen = 0;
    int index = picVec.size() - 1;
    auto pic = picVec[index];
    remainLen += pic->GetPicWidth();
    --index;

    while (index >= m_curIndex) {
        pic = picVec[index];
        remainLen += pic->GetPicWidth();
        --index;
    }
    return remainLen;
}

void PicPlayerMovieByScene::SetGeometryCallback(std::shared_ptr<PicRenderForDraw> picDrawPtr)
{

}

void PicPlayerMovieByScene::SetPicInfoToComponent(int index)
{
    std::vector<std::shared_ptr<PicRenderForDraw>> picVec;
    for (auto iterPic = m_picList.begin(); iterPic != m_picList.end(); ++iterPic) {
        picVec.push_back(*iterPic);
    }

    if (picVec.empty() || index < 0 || index >= static_cast<int>(picVec.size()))
        return;

    auto curPic = picVec[index];
    float scale = curPic->GetShowScale();
    double displayWidth = static_cast<double>(curPic->GetPicWidth());
    double displayPos = m_picMovePos * scale;

    // 避免未布局或宽度异常时误触发
    const double kMinDisplayWidth = 10.0;
    if (displayWidth < kMinDisplayWidth || ceil(displayPos) < displayWidth)
        return;

    // 必须已经滚过该图才回调：要么已切到下一张（m_curIndex > index），要么是最后一张且已滚完
    bool hasScrolledPast = (m_curIndex > index) || (index == static_cast<int>(picVec.size()) - 1);
    if (!hasScrolledPast)
        return;

    std::shared_ptr<PicRenderForDraw> fullyDisplayedPic = GetPicDrawPtr(index);
    if (!fullyDisplayedPic)
        return;

    std::string fullyDisplayedShowId = fullyDisplayedPic->GetPicId();
    if (fullyDisplayedShowId.empty())
        return;

    if (fullyDisplayedShowId == m_curShowid)
        return;

    m_curShowid = fullyDisplayedShowId;
    LOG_DEBUG("SetPicInfoToComponent: picture fully displayed and scrolled past, showId='{}' (index={})", fullyDisplayedShowId, index);
    OnCurPicChange(fullyDisplayedShowId);
}

