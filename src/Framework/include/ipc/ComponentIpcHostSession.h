#ifndef FRAMEWORK_COMPONENT_IPC_HOST_SESSION_H
#define FRAMEWORK_COMPONENT_IPC_HOST_SESSION_H

#include "ComponentIpcMessage.h"

#include <QObject>
#include <QPointer>
#include <QString>

class QLocalServer;
class QLocalSocket;
class QTimer;

class ComponentIpcHostSession : public QObject
{
    Q_OBJECT
public:
    explicit ComponentIpcHostSession(QObject* parent = nullptr);
    ~ComponentIpcHostSession() override;

    bool start(const QString& componentId);
    void stop();

    QString endpointName() const { return m_endpointName; }
    QString token() const { return m_token; }
    bool isClientReady() const { return m_clientReady; }

    bool sendNotification(const QString& method, const QJsonObject& payload);

signals:
    void clientReady();
    void clientDisconnected();
    void notificationReceived(const QString& method, const QJsonObject& payload);
    void eventReceived(const QString& method, const QJsonObject& payload);

private slots:
    void onNewConnection();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onHeartbeatTick();

private:
    bool sendMessage(const ComponentIpcMessage& msg);
    bool handleIncomingMessage(const ComponentIpcMessage& msg);
    void resetSocket();

private:
    QLocalServer* m_server = nullptr;
    QPointer<QLocalSocket> m_socket;
    QString m_endpointName;
    QString m_token;
    QString m_componentId;
    bool m_clientReady = false;
    QByteArray m_readBuffer;
    QTimer* m_heartbeatTimer = nullptr;
    qint64 m_lastPongMs = 0;
};

#endif // FRAMEWORK_COMPONENT_IPC_HOST_SESSION_H
