#ifndef PICRENDERFORDRAW_H
#define PICRENDERFORDRAW_H

#include "PicGeometry.h"

class PicRenderForDraw
{
public:
    PicRenderForDraw(const std::string& picId);
    ~PicRenderForDraw();

    const std::string& GetPicId() const { return m_ImageId; }
    std::shared_ptr<PicGeometry> GetPicGeoPtr() const { return m_imageGeo; }
    void SetPicInfo(const PicShowInfo& data);
    int GetPicWidth() const;
    int GetPicHeight() const;

protected:
    uint32_t m_imageTime;
    uint32_t m_moveSpeed;
    std::string m_ImageId;
    std::shared_ptr<PicGeometry> m_imageGeo;
};

#endif // PICRENDERFORDRAW_H
