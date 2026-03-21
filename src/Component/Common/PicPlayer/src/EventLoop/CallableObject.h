#ifndef CALLABLEOBJECT_H
#define CALLABLEOBJECT_H

#include <functional>

struct CallableObject
{
    CallableObject(std::function<void(void)> f)
        :fn(f)    {}

    void call()
    {
        if (fn){
            fn();
        }
    }
    std::function<void(void)> fn;
};

template<typename F, typename... Args>
CallableObject* CreateCallableObj(F&& f, Args&& ...args)
{
    return new CallableObject(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
}

#endif // CALLABLEOBJECT_H

