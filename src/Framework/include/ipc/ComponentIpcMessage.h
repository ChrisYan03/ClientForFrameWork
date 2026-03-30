#ifndef FRAMEWORK_COMPONENT_IPC_MESSAGE_H
#define FRAMEWORK_COMPONENT_IPC_MESSAGE_H

#include <QJsonObject>
#include <QString>

struct ComponentIpcMessage
{
    QString type; // hello / notification / event / ack / error
    QString method;
    QJsonObject payload;
    QString requestId;
    QString error;
    QString token;
    QString componentId;
    int protocolVersion = 1;

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject& obj, ComponentIpcMessage& out);
};

#endif // FRAMEWORK_COMPONENT_IPC_MESSAGE_H
