#include "../../../include/ipc/ComponentIpcClient.h"

#include "LogUtil.h"

#include <QJsonDocument>
#include <QLocalSocket>

ComponentIpcClient::ComponentIpcClient(QObject* parent)
    : QObject(parent)
{
}

ComponentIpcClient::~ComponentIpcClient()
{
    disconnectFromHost();
}

bool ComponentIpcClient::connectToHost(const QString& endpointName, const QString& token, const QString& componentId, int timeoutMs)
{
    disconnectFromHost();
    auto* socket = new QLocalSocket(this);
    m_socket = socket;
    m_token = token;
    m_componentId = componentId;
    connect(socket, &QLocalSocket::readyRead, this, &ComponentIpcClient::onReadyRead);
    connect(socket, &QLocalSocket::disconnected, this, &ComponentIpcClient::onDisconnected);
    socket->connectToServer(endpointName);
    if (!socket->waitForConnected(timeoutMs)) {
        LOG_ERROR("ComponentIpcClient: connect failed endpoint={} err={}",
            endpointName.toStdString(),
            socket->errorString().toStdString());
        disconnectFromHost();
        return false;
    }

    ComponentIpcMessage hello;
    hello.type = QStringLiteral("hello");
    hello.method = QStringLiteral("hello");
    hello.token = m_token;
    hello.componentId = m_componentId;
    hello.protocolVersion = 1;
    if (!sendMessage(hello)) {
        disconnectFromHost();
        return false;
    }
    return true;
}

void ComponentIpcClient::disconnectFromHost()
{
    if (m_socket) {
        m_socket->disconnect(this);
        m_socket->disconnectFromServer();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    m_readBuffer.clear();
    m_token.clear();
    m_componentId.clear();
}

bool ComponentIpcClient::isConnected() const
{
    return m_socket && m_socket->state() == QLocalSocket::ConnectedState;
}

bool ComponentIpcClient::sendNotification(const QString& method, const QJsonObject& payload)
{
    ComponentIpcMessage msg;
    msg.type = QStringLiteral("notification");
    msg.method = method;
    msg.payload = payload;
    msg.token = m_token;
    msg.componentId = m_componentId;
    return sendMessage(msg);
}

bool ComponentIpcClient::sendEvent(const QString& method, const QJsonObject& payload)
{
    ComponentIpcMessage msg;
    msg.type = QStringLiteral("event");
    msg.method = method;
    msg.payload = payload;
    msg.token = m_token;
    msg.componentId = m_componentId;
    return sendMessage(msg);
}

void ComponentIpcClient::onReadyRead()
{
    if (!m_socket)
        return;
    m_readBuffer.append(m_socket->readAll());
    int pos = -1;
    while ((pos = m_readBuffer.indexOf('\n')) >= 0) {
        const QByteArray line = m_readBuffer.left(pos).trimmed();
        m_readBuffer.remove(0, pos + 1);
        if (line.isEmpty())
            continue;
        QJsonParseError err;
        const QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject())
            continue;
        ComponentIpcMessage msg;
        if (ComponentIpcMessage::fromJson(doc.object(), msg))
            handleIncomingMessage(msg);
    }
}

void ComponentIpcClient::onDisconnected()
{
    emit disconnected();
}

bool ComponentIpcClient::sendMessage(const ComponentIpcMessage& msg)
{
    if (!isConnected())
        return false;
    const QJsonDocument doc(msg.toJson());
    QByteArray out = doc.toJson(QJsonDocument::Compact);
    out.append('\n');
    return m_socket->write(out) == out.size() && m_socket->flush();
}

bool ComponentIpcClient::handleIncomingMessage(const ComponentIpcMessage& msg)
{
    if (msg.type == QStringLiteral("notification") && msg.method == QStringLiteral("framework.ping")) {
        QJsonObject payload;
        payload.insert(QStringLiteral("ts"), msg.payload.value(QStringLiteral("ts")).toVariant().toLongLong());
        sendEvent(QStringLiteral("component.pong"), payload);
        return true;
    }
    if (msg.type == QStringLiteral("notification")) {
        emit notificationReceived(msg.method, msg.payload);
        return true;
    }
    return msg.type == QStringLiteral("ack");
}
