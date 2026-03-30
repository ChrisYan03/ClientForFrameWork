#include "../../../include/ipc/ComponentIpcMessage.h"

QJsonObject ComponentIpcMessage::toJson() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("type"), type);
    obj.insert(QStringLiteral("method"), method);
    obj.insert(QStringLiteral("payload"), payload);
    if (!requestId.isEmpty())
        obj.insert(QStringLiteral("requestId"), requestId);
    if (!error.isEmpty())
        obj.insert(QStringLiteral("error"), error);
    obj.insert(QStringLiteral("token"), token);
    obj.insert(QStringLiteral("componentId"), componentId);
    obj.insert(QStringLiteral("protocolVersion"), protocolVersion);
    return obj;
}

bool ComponentIpcMessage::fromJson(const QJsonObject& obj, ComponentIpcMessage& out)
{
    const QString typeVal = obj.value(QStringLiteral("type")).toString();
    if (typeVal.isEmpty())
        return false;
    out.type = typeVal;
    out.method = obj.value(QStringLiteral("method")).toString();
    out.payload = obj.value(QStringLiteral("payload")).toObject();
    out.requestId = obj.value(QStringLiteral("requestId")).toString();
    out.error = obj.value(QStringLiteral("error")).toString();
    out.token = obj.value(QStringLiteral("token")).toString();
    out.componentId = obj.value(QStringLiteral("componentId")).toString();
    out.protocolVersion = obj.value(QStringLiteral("protocolVersion")).toInt(1);
    return true;
}
