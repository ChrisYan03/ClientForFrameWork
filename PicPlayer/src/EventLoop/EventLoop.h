#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "CallableObject.h"
#include "EventMessage.h"
#include <queue>
#include <atomic>
#include <mutex>

class EventLoop
{
public:
    EventLoop();

    void quit();
    int exec();
    void post(std::shared_ptr<IEventMessage> message);

    template<typename F, typename... Args>
    void asyncInvokeAny(F&& f, Args&& ...args)
    {
        post(std::shared_ptr<IEventMessage>(CallableEventMessage::Create(std::forward<F>(f), std::forward<Args>(args)...)));
    }

    template<typename F, typename... Args>
    void idleRun(F&& f, Args&& ...args)
    {
        m_idleCallback = std::shared_ptr<CallableObject>(CreateCallableObj(std::forward<F>(f), std::forward<Args>(args)...));
    }

protected:
    void idle();

protected:
    std::queue<std::shared_ptr<IEventMessage>> m_messageQueue;
    std::atomic_bool m_stop;
    std::condition_variable m_condition;
    std::mutex m_mutex;
    std::shared_ptr<CallableObject> m_idleCallback;
};

#endif // EVENTLOOP_H

