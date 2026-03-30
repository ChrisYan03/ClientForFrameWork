#include "../../../include/ipc/ComponentIpcHostSession.h"

#include "LogUtil.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QJsonDocument>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>
#include <QUuid>

ComponentIpcHostSession::ComponentIpcHostSession(QObject* parent)
    : QObject(parent)
{
    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(5000);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &ComponentIpcHostSession::onHeartbeatTick);
}

ComponentIpcHostSession::~ComponentIpcHostSession()
{
    stop();
}

bool ComponentIpcHostSession::start(const QString& componentId)
{
    stop();

    m_componentId = componentId;
    m_token = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_endpointName = QStringLiteral("cff_%1_%2_%3")
        .arg(componentId)
        .arg(QCoreApplication::applicationPid())
        .arg(QDateTime::currentMSecsSinceEpoch());

    m_server = new QLocalServer(this);
    connect(m_server, &QLocalServer::newConnection, this, &ComponentIpcHostSession::onNewConnection);
    QLocalServer::removeServer(m_endpointName);
    if (!m_server->listen(m_endpointName)) {
        LOG_ERROR("ComponentIpcHostSession: listen failed endpoint={} err={}",
            m_endpointName.toStdString(),
            m_server->errorString().toStdString());
        stop();
        return false;
    }
    return true;
}

void ComponentIpcHostSession::stop()
{
    resetSocket();
    if (m_server) {
        if (m_server->isListening())
            m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }
    if (!m_endpointName.isEmpty())
        QLocalServer::removeServer(m_endpointName);
    m_endpointName.clear();
    m_token.clear();
    m_componentId.clear();
    m_clientReady = false;
    m_readBuffer.clear();
}

bool ComponentIpcHostSession::sendNotification(const QString& method, const QJsonObject& payload)
{
    ComponentIpcMessage msg;
    msg.type = QStringLiteral("notification");
    msg.method = method;
    msg.payload = payload;
    msg.token = m_token;
    msg.componentId = m_componentId;
    msg.protocolVersion = 1;
    return sendMessage(msg);
}

void ComponentIpcHostSession::onNewConnection()
{
    if (!m_server)
        return;
    QLocalSocket* socket = m_server->nextPendingConnection();
    if (!socket)
        return;
    resetSocket();
    m_socket = socket;
    m_clientReady = false;
    connect(socket, &QLocalSocket::readyRead, this, &ComponentIpcHostSession::onSocketReadyRead);
    connect(socket, &QLocalSocket::disconnected, this, &ComponentIpcHostSession::onSocketDisconnected);
}

void ComponentIpcHostSession::onSocketReadyRead()
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

void ComponentIpcHostSession::onSocketDisconnected()
{
    const bool wasReady = m_clientReady;
    resetSocket();
    if (wasReady)
        emit clientDisconnected();
}

bool ComponentIpcHostSession::sendMessage(const ComponentIpcMessage& msg)
{
    if (!m_socket || m_socket->state() != QLocalSocket::ConnectedState)
        return false;
    const QJsonDocument doc(msg.toJson());
    QByteArray out = doc.toJson(QJsonDocument::Compact);
    out.append('\n');
    return m_socket->write(out) == out.size() && m_socket->flush();
}

bool ComponentIpcHostSession::handleIncomingMessage(const ComponentIpcMessage& msg)
{
    if (msg.type == QStringLiteral("hello")) {
        if (msg.token != m_token || msg.componentId != m_componentId)
            return false;
        m_clientReady = true;
        m_lastPongMs = QDateTime::currentMSecsSinceEpoch();
        ComponentIpcMessage ack;
        ack.type = QStringLiteral("ack");
        ack.method = QStringLiteral("hello");
        ack.token = m_token;
        ack.componentId = m_componentId;
        sendMessage(ack);
        if (m_heartbeatTimer)
            m_heartbeatTimer->start();
        emit clientReady();
        return true;
    }
    if (!m_clientReady)
        return false;
    if (msg.type == QStringLiteral("event")) {
        if (msg.method == QStringLiteral("component.pong")) {
            m_lastPongMs = QDateTime::currentMSecsSinceEpoch();
            return true;
        }
        emit eventReceived(msg.method, msg.payload);
        return true;
    }
    if (msg.type == QStringLiteral("notification")) {
        emit notificationReceived(msg.method, msg.payload);
        return true;
    }
    return false;
}

void ComponentIpcHostSession::resetSocket()
{
    if (m_heartbeatTimer)
        m_heartbeatTimer->stop();
    if (m_socket) {
        m_socket->disconnect(this);
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    m_clientReady = false;
    m_lastPongMs = 0;
    m_readBuffer.clear();
}

void ComponentIpcHostSession::onHeartbeatTick()
{
    if (!m_clientReady)
        return;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_lastPongMs > 0 && (now - m_lastPongMs) > 15000) {
        LOG_WARN("ComponentIpcHostSession: heartbeat timeout component={}", m_componentId.toStdString());
        if (m_socket)
            m_socket->disconnectFromServer();
        return;
    }
    QJsonObject payload;
    payload.insert(QStringLiteral("ts"), now);
    sendNotification(QStringLiteral("framework.ping"), payload);
}
