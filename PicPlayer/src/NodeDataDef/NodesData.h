#ifndef NODESDATA_H
#define NODESDATA_H

#include <list>
#include <atomic>
#include <memory>

class RenderComData
{
public:
    enum RenderComType {
        UserRenderTypr = 1,
    };
    RenderComData() {}
    virtual ~RenderComData() {}

    virtual int RenderType() const = 0;
    virtual bool IsSame(const RenderComData* other) { return false; }
};

class RenderNodesData
{
public:
    RenderNodesData();
    virtual ~RenderNodesData();

    inline std::atomic_bool& Dirty() { return m_dirty; }

    void AppendComData(std::unique_ptr<RenderComData>&& dataPtr);
    std::list<std::unique_ptr<RenderComData>>& GetComDataList();
    void ClearCacheComData();

protected:
    std::atomic_bool m_dirty;
    std::list<std::unique_ptr<RenderComData>> m_dataList;
};

#endif // PICPLAYERRENDER_H
