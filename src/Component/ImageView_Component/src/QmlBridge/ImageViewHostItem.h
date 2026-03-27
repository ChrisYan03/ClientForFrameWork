#ifndef IMAGEVIEWHOSTITEM_H
#define IMAGEVIEWHOSTITEM_H

#include <QQuickItem>
#include <QProcess>

class QWindow;

class ImageViewHostItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)

public:
    explicit ImageViewHostItem(QQuickItem* parent = nullptr);
    ~ImageViewHostItem() override;

    static void setComponentBasePath(const QString& basePath);

    bool running() const { return m_running; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void toggle();

signals:
    void runningChanged();

protected:
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private:
    void ensureHostWindowCreated();
    void updateHostWindowGeometry();
    QString hostProgramPath() const;
    QStringList hostProgramArgs() const;

    static QString s_componentBasePath;

    QWindow* m_hostWindow = nullptr;
    QProcess* m_process = nullptr;
    bool m_running = false;
    bool m_stopping = false;
};

#endif // IMAGEVIEWHOSTITEM_H
