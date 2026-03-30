#ifndef FRAMEWORK_COMPONENT_IPC_CLIENT_H
#define FRAMEWORK_COMPONENT_IPC_CLIENT_H

#include "ComponentIpcMessage.h"

#include <QObject>
#include <QPointer>

class QLocalSocket;

class ComponentIpcClient : public QObject
{
    Q_OBJECT
public:
    explicit ComponentIpcClient(QObject* parent = nullptr);
    ~ComponentIpcClient() override;

    bool connectToHost(const QString& endpointName, const QString& token, const QString& componentId, int timeoutMs = 3000);
    void disconnectFromHost();
    bool isConnected() const;

    bool sendNotification(const QString& method, const QJsonObject& payload);
    bool sendEvent(const QString& method, const QJsonObject& payload);

signals:
    void notificationReceived(const QString& method, const QJsonObject& payload);
    void disconnected();

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    bool sendMessage(const ComponentIpcMessage& msg);
    bool handleIncomingMessage(const ComponentIpcMessage& msg);

private:
    QPointer<QLocalSocket> m_socket;
    QString m_token;
    QString m_componentId;
    QByteArray m_readBuffer;
};

#endif // FRAMEWORK_COMPONENT_IPC_CLIENT_H
