#include "EventLoop.h"

EventLoop::EventLoop()
{

}

void EventLoop::quit()
{
    m_stop = true;
}

int EventLoop::exec()
{
    m_stop = false;
    while (!m_stop) {
        bool timeout;
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            timeout = m_condition.wait_for(ulock, std::chrono::milliseconds(10), [this](){
                return !m_messageQueue.empty();
            });
        }
        if (timeout){
            m_mutex.lock();
            while (!m_messageQueue.empty()) {
                std::shared_ptr<IEventMessage> msg = m_messageQueue.front();
                m_messageQueue.pop();
                m_mutex.unlock();
                if (m_stop)
                    return 0;
                if (msg)
                    msg->Invoke();
                 m_mutex.lock();
            }
            m_mutex.unlock();
        } else {
            idle();
        }
    }
    return 0;
}

void EventLoop::post(std::shared_ptr<IEventMessage> message)
{
    std::unique_lock<std::mutex> ulock(m_mutex);
    m_messageQueue.push(message);
    m_condition.notify_all();
}

void EventLoop::idle()
{
    if (m_idleCallback) {
        m_idleCallback->call();
    }
}

