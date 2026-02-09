#include "PicRenderForDraw.h"


PicRenderForDraw::PicRenderForDraw(const std::string& picId)
    : m_imageTime(0)
    , m_moveSpeed(4)
    , m_scale(1.0)
    , m_ImageId(picId)
{
    m_imageGeo = std::make_shared<PicGeometry>();
}

PicRenderForDraw::~PicRenderForDraw()
{

}

void PicRenderForDraw::SetPicShowScale(float displayHeight)
{
    m_scale = m_imageGeo->GetPicContentScale(displayHeight);
}


void PicRenderForDraw::SetPicInfo(std::shared_ptr<PicShowInfo> data)
{
    m_imageTime = data->picReadTime;
    m_imageGeo->AddNewPic(data);
}

int PicRenderForDraw::GetPicWidth() const
{
    return m_imageGeo->GetPicWidth();
}

int PicRenderForDraw::GetPicContentHeight() const
{
    return m_imageGeo->GetPicHeight() * m_scale;
}

