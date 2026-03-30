#include "ImageViewHostItem.h"
#include "ipc/ComponentIpcHostSession.h"
#include <QQuickWindow>
#include <QWindow>
#include <QQuickItem>
#include <QFileInfo>
#include <QFile>
#include <QTimer>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QIcon>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLocale>
#include "LogUtil.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

QString ImageViewHostItem::s_componentBasePath;

namespace {
constexpr int kMainButtonSize = 64;
constexpr int kCircleButtonSize = 64;
constexpr int kMainRightMargin = 24;
constexpr int kMainTopMargin = 12;
constexpr int kCircleRightMargin = 24;
constexpr int kCircleBottomMargin = 24;
constexpr int kCircleSpacing = 36;
const char* kCircleQssPath = ":/ImageView/styles/overlay_circle_button.qss";
const char* kIconStart = ":/ImageView/icons/preview_start.svg";
const char* kIconPause = ":/ImageView/icons/preview_pause.svg";
const char* kIconHeart = ":/ImageView/icons/preview_heart.svg";
const char* kIconClose = ":/ImageView/icons/preview_close.svg";
}

ImageViewHostItem::ImageViewHostItem(QQuickItem* parent)
    : QQuickItem(parent)
{
    m_ipcSession = new ComponentIpcHostSession(this);
    setFlag(ItemHasContents, false);
    connect(this, &QQuickItem::windowChanged, this, &ImageViewHostItem::onWindowChanged);
    connect(this, &ImageViewHostItem::runningChanged, this, &ImageViewHostItem::updateButtonStyleAndText);
    connect(m_ipcSession, &ComponentIpcHostSession::clientReady, this, [this]() {
        sendIpcFullState();
    });
    connect(m_ipcSession, &ComponentIpcHostSession::eventReceived, this, [this](const QString& method, const QJsonObject&) {
        if (method == QStringLiteral("component.state.running")) {
            if (!m_running) {
                m_running = true;
                emit runningChanged();
            }
        } else if (method == QStringLiteral("component.state.stopped")) {
            if (m_running) {
                m_running = false;
                emit runningChanged();
            }
        }
    });
}

void ImageViewHostItem::setFrameworkLanguage(int lang)
{
    if (m_frameworkLanguage == lang)
        return;
    m_frameworkLanguage = lang;
    emit frameworkLanguageChanged();
    loadI18nAndThemeResources();
    updateButtonStyleAndText();
    if (m_ipcSession && m_ipcSession->isClientReady()) {
        QJsonObject payload;
        payload.insert(QStringLiteral("language"), m_frameworkLanguage);
        m_ipcSession->sendNotification(QStringLiteral("framework.sync.language"), payload);
    }
}

void ImageViewHostItem::applyTheme(const QVariantMap& themeColors)
{
    m_frameworkThemeColors = themeColors;
    if (m_ipcSession && m_ipcSession->isClientReady()) {
        QJsonObject themeObj;
        for (auto it = m_frameworkThemeColors.constBegin(); it != m_frameworkThemeColors.constEnd(); ++it) {
            if (it.value().canConvert<QString>())
                themeObj.insert(it.key(), it.value().toString());
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("theme"), themeObj);
        m_ipcSession->sendNotification(QStringLiteral("framework.sync.theme"), payload);
    }
}

QString ImageViewHostItem::overlayI18nJsonPath() const
{
    if (m_frameworkLanguage == 0)
        return QStringLiteral(":/ImageView/i18n/overlay_zh_CN.json");
    if (m_frameworkLanguage == 1)
        return QStringLiteral(":/ImageView/i18n/overlay_en_US.json");
    if (QLocale().name().startsWith(QStringLiteral("zh"), Qt::CaseInsensitive))
        return QStringLiteral(":/ImageView/i18n/overlay_zh_CN.json");
    return QStringLiteral(":/ImageView/i18n/overlay_en_US.json");
}

void ImageViewHostItem::onWindowChanged(QQuickWindow* win)
{
    if (!win) {
        return;
    }
    if (m_boundWindow != win) {
        m_boundWindow = win;
        connect(win, &QWindow::xChanged, this, &ImageViewHostItem::syncOverlayGeometry, Qt::UniqueConnection);
        connect(win, &QWindow::yChanged, this, &ImageViewHostItem::syncOverlayGeometry, Qt::UniqueConnection);
        connect(win, &QWindow::widthChanged, this, &ImageViewHostItem::syncOverlayGeometry, Qt::UniqueConnection);
        connect(win, &QWindow::heightChanged, this, &ImageViewHostItem::syncOverlayGeometry, Qt::UniqueConnection);
    }
    QTimer::singleShot(100, this, [this]() {
        ensureHostWindowCreated();
        ensureButtonWidgetCreated();
        updateButtonStyleAndText();
        syncOverlayGeometry();
    });
}

void ImageViewHostItem::syncOverlayGeometry()
{
    updateHostWindowGeometry();
    updateButtonWidgetGeometry();
}

ImageViewHostItem::~ImageViewHostItem()
{
    stop();
    if (m_ipcSession)
        m_ipcSession->stop();
    for (const auto& it : m_buttonWidgets) {
        if (it.second) {
            it.second->hide();
            it.second->deleteLater();
        }
    }
    m_buttonWidgets.clear();
    m_buttonControls.clear();
    if (m_hostWindow) {
        m_hostWindow->destroy();
        m_hostWindow->deleteLater();
        m_hostWindow = nullptr;
    }
}

void ImageViewHostItem::setComponentBasePath(const QString& basePath)
{
    s_componentBasePath = basePath;
}

void ImageViewHostItem::start()
{
    if (m_running) {
        return;
    }
    if (m_stopping || (m_process && m_process->state() != QProcess::NotRunning)) {
        return;
    }
    ensureHostWindowCreated();
    if (!m_hostWindow) {
        LOG_WARN("ImageViewHostItem: host window is null, cannot start process");
        return;
    }
    m_hostWindow->show();
    ensureButtonWidgetCreated();
    setButtonWidgetVisible(OverlayButtonType::MainToggle, true);
    setButtonWidgetVisible(OverlayButtonType::Like, true);
    setButtonWidgetVisible(OverlayButtonType::Close, true);
    raiseButtonWidget(OverlayButtonType::MainToggle);
    raiseButtonWidget(OverlayButtonType::Like);
    raiseButtonWidget(OverlayButtonType::Close);

    const QString program = hostProgramPath();
    if (!QFileInfo::exists(program)) {
        LOG_WARN("ImageViewHostItem: player host executable not found: {}", program.toStdString());
        return;
    }

    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
                [this](int, QProcess::ExitStatus) {
                    if (m_ipcSession)
                        m_ipcSession->stop();
                    m_stopping = false;
                    m_running = false;
                    emit runningChanged();
                });
    }

    if (!m_ipcSession->start(QStringLiteral("ImageView"))) {
        LOG_ERROR("ImageViewHostItem: failed to start IPC host session");
        return;
    }

    syncOverlayGeometry();
    m_process->start(program, hostProgramArgs());
    if (!m_process->waitForStarted(3000)) {
        if (m_ipcSession)
            m_ipcSession->stop();
        LOG_ERROR("ImageViewHostItem: failed to start process: {}", m_process->errorString().toStdString());
        return;
    }

    m_running = true;
    emit runningChanged();
}

void ImageViewHostItem::stop()
{
    if (m_ipcSession && m_ipcSession->isClientReady()) {
        m_ipcSession->sendNotification(QStringLiteral("framework.lifecycle.stop"), QJsonObject{});
    }
    if (m_hostWindow) {
        m_hostWindow->hide();
    }
    if (!isVisible()) {
        setButtonWidgetVisible(OverlayButtonType::MainToggle, false);
        setButtonWidgetVisible(OverlayButtonType::Like, false);
        setButtonWidgetVisible(OverlayButtonType::Close, false);
    }

    if (!m_process || !m_running) {
        if (m_hostWindow) {
            m_hostWindow->hide();
        }
        m_stopping = false;
        return;
    }

    m_running = false;
    emit runningChanged();
    m_stopping = true;

    m_process->terminate();
    QPointer<QProcess> processGuard(m_process);
    QTimer::singleShot(1500, this, [processGuard]() {
        if (processGuard && processGuard->state() != QProcess::NotRunning) {
            processGuard->kill();
        }
    });
}

void ImageViewHostItem::toggle()
{
    onOverlayButtonClicked(OverlayButtonType::MainToggle);
}

void ImageViewHostItem::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    syncOverlayGeometry();
}

bool ImageViewHostItem::isDarkMode() const
{
    const QVariant themeProp = qApp ? qApp->property("appTheme") : QVariant();
    if (themeProp.isValid() && themeProp.canConvert<int>())
        return themeProp.toInt() == 1;
    return false;
}

void ImageViewHostItem::ensureHostWindowCreated()
{
    if (m_hostWindow || !window()) {
        return;
    }
    m_hostWindow = new QWindow(window());
    m_hostWindow->setFlags(Qt::Widget);
    m_hostWindow->setParent(window());
    m_hostWindow->hide();
}

void ImageViewHostItem::updateHostWindowGeometry()
{
    if (!m_hostWindow || !window()) {
        return;
    }
    const QPointF scenePos = mapToScene(QPointF(0, 0));
    m_hostWindow->setGeometry(qRound(scenePos.x()), qRound(scenePos.y()), qRound(width()), qRound(height()));
}

void ImageViewHostItem::ensureButtonWidgetCreated()
{
    if (widgetForButton(OverlayButtonType::MainToggle) &&
        widgetForButton(OverlayButtonType::Like) &&
        widgetForButton(OverlayButtonType::Close)) {
        return;
    }
    if (!window()) {
        return;
    }

    loadI18nAndThemeResources();
    const QString circleQssTemplate = loadTextResource(QString::fromUtf8(kCircleQssPath));

    const auto createOverlayButton = [this, &circleQssTemplate](OverlayButtonType type, int size, int iconSize, const QString& iconPath) {
        QWidget* container = new QWidget();
        container->setFixedSize(size, size);
        container->setAutoFillBackground(false);

        QPushButton* btn = new QPushButton(container);
        btn->setFixedSize(size, size);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setFlat(true);
        btn->setText(QString());
        btn->setIconSize(QSize(iconSize, iconSize));
        btn->setIcon(QIcon(iconPath));
        connect(btn, &QPushButton::clicked, this, [this, type]() { onOverlayButtonClicked(type); });

        QVBoxLayout* layout = new QVBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(btn);
        container->setLayout(layout);
        container->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus | Qt::WindowStaysOnTopHint);
        container->setAttribute(Qt::WA_ShowWithoutActivating, true);
        container->setAttribute(Qt::WA_TranslucentBackground, true);
        container->setAttribute(Qt::WA_NoSystemBackground, true);
        container->setStyleSheet(QStringLiteral("background: transparent; border: none;"));

        m_buttonWidgets[type] = container;
        m_buttonControls[type] = btn;
        Q_UNUSED(circleQssTemplate);
    };

    createOverlayButton(OverlayButtonType::MainToggle, kMainButtonSize, 24, QString::fromUtf8(kIconStart));
    createOverlayButton(OverlayButtonType::Like, kCircleButtonSize, 24, QString::fromUtf8(kIconHeart));
    createOverlayButton(OverlayButtonType::Close, kCircleButtonSize, 24, QString::fromUtf8(kIconClose));
}

void ImageViewHostItem::updateButtonWidgetGeometry()
{
    if (!window()) {
        return;
    }

    QWidget* mainWidget = widgetForButton(OverlayButtonType::MainToggle);
    QWidget* likeWidget = widgetForButton(OverlayButtonType::Like);
    QWidget* closeWidget = widgetForButton(OverlayButtonType::Close);
    if (!mainWidget || !likeWidget || !closeWidget) {
        return;
    }

    const QPointF scenePos = mapToScene(QPointF(0, 0));
    const int topX = qRound(scenePos.x() + width() - kMainButtonSize - kMainRightMargin);
    const int topY = qRound(scenePos.y() + kMainTopMargin);
    const int bottomRightX = qRound(scenePos.x() + width() - kCircleButtonSize - kCircleRightMargin);
    const int bottomY = qRound(scenePos.y() + height() - kCircleButtonSize - kCircleBottomMargin);
    const int bottomLeftX = bottomRightX - kCircleButtonSize - kCircleSpacing;

    mainWidget->move(window()->mapToGlobal(QPoint(topX, topY)));
    closeWidget->move(window()->mapToGlobal(QPoint(bottomRightX, bottomY)));
    likeWidget->move(window()->mapToGlobal(QPoint(bottomLeftX, bottomY)));

    mainWidget->show();
    closeWidget->show();
    likeWidget->show();
    mainWidget->raise();
    closeWidget->raise();
    likeWidget->raise();
}

void ImageViewHostItem::updateButtonStyleAndText()
{
    loadI18nAndThemeResources();
    const QString qssTemplate = loadTextResource(QString::fromUtf8(kCircleQssPath));
    const QString qss = qssTemplate.isEmpty()
        ? QStringLiteral(
            "QPushButton { border: none; border-radius: %1px; background: %2; padding: 0px; }"
            "QPushButton:hover { background: %3; }"
            "QPushButton:pressed { background: %4; }")
        : qssTemplate;

    QPushButton* mainBtn = buttonControl(OverlayButtonType::MainToggle);
    QPushButton* likeBtn = buttonControl(OverlayButtonType::Like);
    QPushButton* closeBtn = buttonControl(OverlayButtonType::Close);
    if (!mainBtn || !likeBtn || !closeBtn) {
        return;
    }

    const QString buttonBg = themeColor("buttonBg", "rgba(60, 64, 67, 0.92)");
    const QString buttonHover = themeColor("buttonHover", "rgba(80, 84, 88, 0.95)");
    const QString buttonPressed = themeColor("buttonPressed", "rgba(43, 46, 49, 0.95)");
    const QString likeBg = themeColor("likeButtonBg", "rgba(220, 53, 69, 0.92)");
    const QString likeHover = themeColor("likeButtonHover", "rgba(231, 76, 94, 0.95)");
    const QString likePressed = themeColor("likeButtonPressed", "rgba(190, 35, 51, 0.95)");

    mainBtn->setStyleSheet(qss.arg(kMainButtonSize / 2).arg(buttonBg).arg(buttonHover).arg(buttonPressed));
    likeBtn->setStyleSheet(qss.arg(kCircleButtonSize / 2).arg(likeBg).arg(likeHover).arg(likePressed));
    closeBtn->setStyleSheet(qss.arg(kCircleButtonSize / 2).arg(buttonBg).arg(buttonHover).arg(buttonPressed));

    mainBtn->setIcon(QIcon(m_running ? QString::fromUtf8(kIconPause) : QString::fromUtf8(kIconStart)));
    likeBtn->setIcon(QIcon(QString::fromUtf8(kIconHeart)));
    closeBtn->setIcon(QIcon(QString::fromUtf8(kIconClose)));

    mainBtn->setToolTip(m_running ? trText("pause", QString::fromUtf8("暂停")) : trText("start", QString::fromUtf8("启动")));
    likeBtn->setToolTip(trText("like", QString::fromUtf8("喜欢")));
    closeBtn->setToolTip(trText("close", QString::fromUtf8("关闭")));
}

QString ImageViewHostItem::loadTextResource(const QString& qrcPath) const
{
    QFile file(qrcPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

void ImageViewHostItem::loadI18nAndThemeResources()
{
    const QString i18nPath = overlayI18nJsonPath();
    const QString themePath = isDarkMode()
        ? QStringLiteral(":/ImageView/themes/overlay_dark.json")
        : QStringLiteral(":/ImageView/themes/overlay_light.json");

    m_i18n.clear();
    m_theme.clear();
    const auto loadObjectToMap = [](const QString& path, QMap<QString, QString>& outMap) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (!doc.isObject()) {
            return;
        }
        const QJsonObject obj = doc.object();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            outMap.insert(it.key(), it.value().toString());
        }
    };
    loadObjectToMap(i18nPath, m_i18n);
    loadObjectToMap(themePath, m_theme);
}

QString ImageViewHostItem::trText(const QString& key, const QString& fallback) const
{
    return m_i18n.value(key, fallback);
}

QString ImageViewHostItem::themeColor(const QString& key, const QString& fallback) const
{
    return m_theme.value(key, fallback);
}

QWidget* ImageViewHostItem::widgetForButton(OverlayButtonType type) const
{
    const auto it = m_buttonWidgets.find(type);
    return it == m_buttonWidgets.end() ? nullptr : it->second.data();
}

QPushButton* ImageViewHostItem::buttonControl(OverlayButtonType type) const
{
    const auto it = m_buttonControls.find(type);
    return it == m_buttonControls.end() ? nullptr : it->second.data();
}

void ImageViewHostItem::setButtonWidgetVisible(OverlayButtonType type, bool visible)
{
    if (QWidget* w = widgetForButton(type)) {
        visible ? w->show() : w->hide();
    }
}

void ImageViewHostItem::raiseButtonWidget(OverlayButtonType type)
{
    if (QWidget* w = widgetForButton(type)) {
        w->raise();
    }
}

void ImageViewHostItem::onOverlayButtonClicked(OverlayButtonType type)
{
    switch (type) {
    case OverlayButtonType::MainToggle:
        m_running ? stop() : start();
        break;
    case OverlayButtonType::Like:
        LOG_INFO("ImageView overlay: like button clicked");
        break;
    case OverlayButtonType::Close:
        stop();
        break;
    }
}

QString ImageViewHostItem::hostProgramPath() const
{
#if defined(Q_OS_WIN)
    return s_componentBasePath + "bin/ImageViewPlayerHost.exe";
#else
    return s_componentBasePath + "bin/ImageViewPlayerHost";
#endif
}

QStringList ImageViewHostItem::hostProgramArgs() const
{
    QStringList args;
    if (m_ipcSession) {
        args << "--ipc-endpoint" << m_ipcSession->endpointName();
        args << "--ipc-token" << m_ipcSession->token();
        args << "--component-id" << QStringLiteral("ImageView");
    }
#if defined(Q_OS_WIN)
    const quintptr wid = m_hostWindow ? static_cast<quintptr>(m_hostWindow->winId()) : 0;
    args << "--parent-hwnd" << QString::number(wid);
    args << "--width" << QString::number(qRound(width()));
    args << "--height" << QString::number(qRound(height()));
#endif
    return args;
}

void ImageViewHostItem::sendIpcFullState()
{
    if (!m_ipcSession || !m_ipcSession->isClientReady())
        return;
    QJsonObject themeObj;
    for (auto it = m_frameworkThemeColors.constBegin(); it != m_frameworkThemeColors.constEnd(); ++it) {
        if (it.value().canConvert<QString>())
            themeObj.insert(it.key(), it.value().toString());
    }
    QJsonObject payload;
    payload.insert(QStringLiteral("theme"), themeObj);
    payload.insert(QStringLiteral("language"), m_frameworkLanguage);
    m_ipcSession->sendNotification(QStringLiteral("framework.sync.fullState"), payload);
}
