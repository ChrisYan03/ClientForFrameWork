#include "NodesDataForDraw.h"

int PicData::RenderType() const
{
    return (int)NodesType::PicDataType;
}

int PicRect::RenderType() const
{
    return (int)NodesType::PicRectType;
}

PicRemove::PicRemove(const std::string& curpicId)
    : picId(curpicId)
{}

int PicRemove::RenderType() const
{
    return (int)NodesType::PicRemoveType;
}

PicShowNow::PicShowNow(const std::string& curpicId)
    : picId(curpicId)
{}

int PicShowNow::RenderType() const
{
    return (int)NodesType::PicChangeType;
}

