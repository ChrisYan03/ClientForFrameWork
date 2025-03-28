#ifndef NODESDATAFORDRAW_H
#define NODESDATAFORDRAW_H

#include "../PicPlayerDataDef.h"
#include "NodesData.h"

enum class NodesType
{
    PicDataType = RenderComData::UserRenderTypr + 1,
    PicRectType,
    PicRemoveType,
    PicChangeType,
};

class PicData : public RenderComData
{
public:
    PicData() {}
    virtual ~PicData() {}

    virtual int RenderType() const  override;

public:
    PicShowInfo picShowData;
};

class PicRect : public RenderComData
{
public:
    PicRect() = default;
    virtual ~PicRect() = default;

    virtual int RenderType() const  override;

public:
    std::string picId;
};

class PicRemove : public RenderComData
{
public:
    PicRemove(const std::string& curpicId);
    virtual ~PicRemove()  = default;

    virtual int RenderType() const  override;

public:
    std::string picId;
};

class PicShowNow : public RenderComData
{
public:
    PicShowNow(const std::string& curpicId);
    virtual ~PicShowNow()  = default;

    virtual int RenderType() const  override;

public:
    std::string picId;
};

#endif // NODESDATAFORDRAW_H
