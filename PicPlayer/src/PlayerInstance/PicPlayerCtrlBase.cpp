#include "PicPlayerCtrlBase.h"

PicPlayerCtrlBase::PicPlayerCtrlBase()
{

}

PicPlayerCtrlBase::~PicPlayerCtrlBase()
{

}

void PicPlayerCtrlBase::SetRenderSync(const std::shared_ptr<PicPlayerRenderSync>& syncPtr)
{
    m_syncPtr = syncPtr;
}
