#ifndef EVENTMESSAGE_H
#define EVENTMESSAGE_H

#include "CallableObject.h"
#include <memory>

class IEventMessage
{
public:
    virtual ~IEventMessage() {};
    virtual void Invoke() = 0;
};

class CallableEventMessage : public IEventMessage
{
public:
    void Invoke() override {
        if (m_callback) {
            m_callback->call();
        }
    }

    template<typename F, typename... Args>
    static CallableEventMessage* Create(F&& f, Args&& ...args)
    {
        CallableEventMessage* message = new CallableEventMessage();
        message->m_callback = std::shared_ptr<CallableObject>(CreateCallableObj(std::forward<F>(f), std::forward<Args>(args)...));
        return message;
    }

private:
    std::shared_ptr<CallableObject> m_callback;
};

#endif // EVENTMESSAGE_H

