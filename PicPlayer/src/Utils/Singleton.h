#ifndef SINGLETON_H
#define SINGLETON_H


#ifdef _WIN32
#include <Windows.h>
#endif
#include <mutex>
#ifdef __cplusplus

template<typename T>
class Singleton
{
public:
    static inline T* instance(){
        std::call_once(m_onceFlag, []() {
            m_instance = new T;
        });
        return m_instance;
    }

    static inline void free(){
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance != nullptr) {
            delete m_instance;
            m_instance = nullptr;
        }
    }

protected:
    Singleton(){}
    virtual ~Singleton(){
        free();
    }

private:
    //拷贝构造函数和赋值操作符声明为私有，以防止拷贝和赋值
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

private:
    static T* m_instance;
    static std::mutex m_mutex;
    static std::once_flag m_onceFlag;
};

template<typename T>
T* Singleton<T>::m_instance = nullptr;

template<typename T>
std::mutex Singleton<T>::m_mutex;

template<typename T>
std::once_flag Singleton<T>::m_onceFlag;

#endif

#endif // HANDLEREGISTER_H
