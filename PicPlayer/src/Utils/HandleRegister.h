#ifndef HANDLEREGISTER_H
#define HANDLEREGISTER_H

#include "IDGenerator.h"
#include <map>
#include <mutex>

template<typename T>
class HandleRegister
{
public:
    HandleRegister<T>() {}
    virtual ~HandleRegister<T>() {}

    inline void InitHandleRegister(){
        m_idHandle.Init(m_maxHandleNum);
    }

    inline void ReleaseAll(){
        m_idHandle.ReleaseAllId();
        m_objInsMap.clear();
    }

    int RegisterObjInstance(std::unique_ptr<T>&& obj_ptr){
        std::lock_guard<std::mutex> guard(m_mutex);
        int id = m_idHandle.GenId();
        if (0 != id){
            m_objInsMap.emplace(id, std::forward<std::unique_ptr<T>>(obj_ptr));
            return id;
        }
        return -1;
    }

    T* GetObjInstance(int handle){
        std::lock_guard<std::mutex> guard(m_mutex);
        auto iter = m_objInsMap.find(handle);
        if (iter != m_objInsMap.end()){
            return iter->second.get();
        }
        return nullptr;
    }

    void EarseObjInstance(int handle){
        m_mutex.lock();
        auto iter = m_objInsMap.find(handle);
        if (iter != m_objInsMap.end()){
            m_idHandle.FreeId(handle);
            std::unique_ptr<T> obj_ptr = std::move(iter->second);
            m_objInsMap.erase(iter);
        }
        m_mutex.unlock();
    }

private:
    std::mutex m_mutex;
    IDGenerator<int> m_idHandle;
    std::map<int, std::unique_ptr<T>> m_objInsMap;
    const int m_maxHandleNum = 200;
};


#endif // HANDLEREGISTER_H

