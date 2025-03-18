#ifndef PICPLAYERCTRLBASE_H
#define PICPLAYERCTRLBASE_H

#include "PicPlayerRenderSync.h"

class PicPlayerCtrlBase
{
public:
    explicit PicPlayerCtrlBase();
    ~PicPlayerCtrlBase();

    void SetRenderSync(const std::shared_ptr<PicPlayerRenderSync>& syncPtr);

protected:
    std::shared_ptr<PicPlayerRenderSync> m_syncPtr;
};

#endif // PICPLAYERCTRLBASE_H
