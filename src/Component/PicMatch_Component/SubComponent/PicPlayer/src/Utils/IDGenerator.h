#ifndef IDGENERATOR_H
#define IDGENERATOR_H

#include <atomic>
#include <list>

template<typename T>
class IDGenerator
{
public:
    IDGenerator() : m_curId(0), m_idMax(0) {}
    ~IDGenerator() {}

    void Init(T idmax){
        m_idMax.store(idmax);
    }

    void ReleaseAllId(){
        m_curId.store(0);
        m_idMax.store(0);
        m_idFreeList.clear();
    }

    void FreeId(T id){
        m_idFreeList.emplace_back(id);
    }

    T GenId(){
        if (IsOverSize()){
            return 0;
        }
        if (!m_idFreeList.empty()){
            T tempId = m_idFreeList.front();
            m_idFreeList.pop_front();
            return tempId;
        }
        return ++m_curId;
    }

private:
    bool IsOverSize()
    {
        return m_curId >= m_idMax;
    }

private:
    std::atomic<T> m_curId;
    std::atomic<T> m_idMax;
    std::list<T>   m_idFreeList;
};

#endif // IDGENERATOR_H

