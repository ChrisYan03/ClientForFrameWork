#ifndef PICRENDERFORDRAW_H
#define PICRENDERFORDRAW_H

#include "PicGeometry.h"
#include <memory> 

class PicRenderForDraw
{
public:
    PicRenderForDraw(const std::string& picId);
    ~PicRenderForDraw();

    const std::string& GetPicId() const { return m_ImageId; }
    double GetShowScale() const { return m_scale; }

    std::shared_ptr<PicGeometry> GetPicGeoPtr() const { return m_imageGeo; }
    void SetPicShowScale(float displayHeight);
    void SetPicInfo(std::shared_ptr<PicShowInfo> data);
    void SetFaceRecogResult(std::shared_ptr<FaceDetectionResult> data);
    int GetPicWidth() const;
    int GetPicContentHeight() const;

protected:
    uint32_t m_imageTime;
    uint32_t m_moveSpeed;
    float m_scale;
    std::string m_ImageId;
    std::shared_ptr<PicGeometry> m_imageGeo;
};

#endif // PICRENDERFORDRAW_H

