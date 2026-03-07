#include "PicMatchWidget.h"
#include "PicPlayerApi.h"
#include "PicRecognitionApi.h"
#include "StbImage/stb_image.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QScrollArea>
#include <QTabWidget>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QSettings>
#include <QFileDialog>
#include <QFrame>
#include <QStackedWidget>
#include "LogUtil.h"
#include <QCoreApplication>
#include <QDir>
#include <QTimer>
#include <QFileInfo>
#include <QFont>
#include <QVariantMap>
#include <memory>

static QString themeColor(const QVariantMap& m, const char* key, const char* defaultHex)
{
    QString v = m.value(QLatin1String(key)).toString();
    return v.isEmpty() ? QLatin1String(defaultHex) : v;
}

static QString applicationBasePath()
{
    QString exeDir = QDir(QCoreApplication::applicationDirPath()).absolutePath();
    return exeDir;
}

/** 组件 bin 目录：主程序从 target/.../Release 运行时在 Component/PicMatch/bin */
static QString componentBinPath()
{
    QString exeDir = QDir(QCoreApplication::applicationDirPath()).absolutePath();
    QString componentBin = exeDir + QStringLiteral("/Component/PicMatch/bin");
    if (QDir(componentBin).exists())
        return componentBin;
    return exeDir;
}

/** 组件图标目录（与 CMake POST_BUILD 复制的 resource/icons 一致），图标从文件加载避免 DLL 内嵌 qrc 链接问题 */
static QString componentIconsPath()
{
    return componentBinPath() + QStringLiteral("/resource/icons");
}

/** 组件模型/数据目录：主程序从 target/.../Release 运行时，数据在 Component/PicMatch/bin */
static QString componentDataPath()
{
    return componentBinPath();
}

static const char* kSettingsOrg = "ClientForFrame";
static const char* kSettingsApp = "PicMatchComponent";
static const char* kSettingsDataPathKey = "DataPath";

PicMatchWidget::PicMatchWidget(BaseWidget *parent)
    : BaseWidget(parent)
    , m_handle(-1)
    , m_playerWidget(nullptr)
    , m_faceShowWidget(nullptr)
    , m_showId("")
    , m_currentIndex(0)
    , m_initialized(false)
    , m_imageNames()
    , m_running(false)
    , m_rightPanel(nullptr)
    , m_buttonBarWidget(nullptr)
    , m_runButton(nullptr)
    , m_stopButton(nullptr)
    , m_configButton(nullptr)
    , m_rightStack(nullptr)
    , m_configPanel(nullptr)
    , m_configPathEdit(nullptr)
{
}

QString PicMatchWidget::getDataPath() const
{
    if (!m_customDataPath.isEmpty() && QDir(m_customDataPath).exists())
        return QDir(m_customDataPath).absolutePath();
    return componentDataPath();
}

PicMatchWidget::~PicMatchWidget()
{
    LOG_DEBUG("~PicMatchWidget");
    Quit();
    DestroyFaceRecognition();
    LOG_DEBUG("~PicMatchWidget suc");
}

void PicMatchWidget::InitUI()
{
    // Main horizontal layout
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(2);

    // Create player widget (left side)
    if (!m_playerWidget) {
        m_playerWidget = new BaseWidget(this);
        m_playerWidget->setObjectName("PicMatchWidget");
        QSizePolicy playerPolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_playerWidget->setSizePolicy(playerPolicy);
    }

    // Create right panel container
    if (!m_rightPanel) {
        m_rightPanel = new QWidget(this);
        m_rightPanel->setObjectName("RightPanel");
        m_rightPanel->setMaximumWidth(350);
        m_rightPanel->setMinimumWidth(250);
    }

    QVBoxLayout* rightLayout = new QVBoxLayout(m_rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    CreateButtonBar(m_rightPanel);
    rightLayout->addWidget(m_buttonBarWidget);

    m_rightStack = new QStackedWidget(m_rightPanel);
    if (!m_faceShowWidget) {
        m_faceShowWidget = new FaceShowWidget(this);
        QSizePolicy facePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_faceShowWidget->setSizePolicy(facePolicy);
    }
    m_rightStack->addWidget(m_faceShowWidget);
    rightLayout->addWidget(m_rightStack, 1);

    // Add widgets to main layout
    mainLayout->addWidget(m_playerWidget, 1); // Player takes remaining space
    mainLayout->addWidget(m_rightPanel);      // Right panel with fixed width

    setLayout(mainLayout);

    // Initialize player layout
    QVBoxLayout* playerLayout = new QVBoxLayout(m_playerWidget);
    playerLayout->setContentsMargins(0, 0, 0, 0);

    loadDataPath();
    // 人脸检测模型（haarcascade/DNN）从组件 bin 目录加载；图片列表仍从 getDataPath()（设置页路径）读取
    InitFaceRecognition(componentBinPath().toLocal8Bit().constData());

    CreateConfigPanel(m_rightStack);

    applyTheme(QVariantMap()); // 使用默认主题色，主框架注册宿主后会再调用 applyTheme 同步当前主题
    UpdateUIState();
}

void PicMatchWidget::InitPicPlayer()
{
    if (nullptr == m_playerWidget) {
        LOG_ERROR("m_playerWidget is nullptr");
        return;
    }
    PicPlayer_Init();
    m_handle = PicPlayer_CreateInstance();
    if (-1 == m_handle) {
        LOG_ERROR("PicPlayer_CreateInstance is failed!!");
        return;
    }
    PicPlayer_RegisterCallback(m_handle, (PlayerMsgCallback)PicCallbackByPlayer, this);
    PicPlayer_RegisterWindow(m_handle, static_cast<Window_ShowID>(m_playerWidget->winId()));
    PicPlayer_Play(m_handle);
}

void PicMatchWidget::Run()
{
    if (!m_running) {
        m_running = true;
        InitPicPlayer();

        m_initialized = false;
        m_currentIndex = 0;
        std::string firstName = GetNextImageName();
        if (!firstName.empty() && firstName != "default_image") {
            const QString basePath = getDataPath();
            const std::string imagePath = (basePath + QChar('/') + QString::fromStdString(firstName)).toStdString();
            if (QFileInfo::exists(QString::fromStdString(imagePath))) {
                const std::string showId = QFileInfo(QString::fromStdString(firstName)).baseName().toStdString();
                UpdatePic(showId, imagePath);
            }
        }

        UpdateUIState();
    }
}

void PicMatchWidget::Quit()
{
    if (!m_running)
        return;
    m_running = false;
    if (m_faceShowWidget) {
        m_faceShowWidget->clearFaceImages(true);
    }
    if (m_handle != -1) {
        PicPlayer_DestroyInstance(m_handle);
        m_handle = -1;
    }
    PicPlayer_UnInit();
    m_showId.clear();
    m_currentIndex = 0;
    m_initialized = false;
    m_imageNames.clear();

    UpdateUIState();
}

void PicMatchWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_handle < 0 || !m_playerWidget)
        return;
#if defined(Q_OS_WIN)
    if (!m_resizeNotifyTimer) {
        m_resizeNotifyTimer = new QTimer(this);
        m_resizeNotifyTimer->setSingleShot(true);
        connect(m_resizeNotifyTimer, &QTimer::timeout, this, [this]() {
            if (m_handle >= 0 && m_playerWidget && m_playerWidget->width() > 0 && m_playerWidget->height() > 0)
                PicPlayer_SetWindowSize(m_handle, m_playerWidget->width(), m_playerWidget->height());
        });
    }
    m_resizeNotifyTimer->start(0);
#else
    if (m_playerWidget->width() > 0 && m_playerWidget->height() > 0)
        QTimer::singleShot(0, this, [this]() {
            if (m_playerWidget)
                PicPlayer_SetWindowSize(m_handle, m_playerWidget->width(), m_playerWidget->height());
        });
#endif
}

void PicMatchWidget::OnRun(const std::string& showid)
{
    if (m_showId != showid) {
        m_showId = showid;
        m_faceShowWidget->clearFaceImages();
        m_faceShowWidget->triggerDisplay();
    }
    else {
        LOG_DEBUG("m_showId == showid; Run PicPlayer size: {} x {}", m_playerWidget->width(), m_playerWidget->height());
        return;
    }
    std::string fileName = GetNextImageName();
    LOG_DEBUG("Run PicPlayer GetNextImageName {}", fileName.c_str());

    if (fileName.empty() || fileName == "default_image") {
        LOG_DEBUG("GetNextImageName returned empty or default, skipping update");
        return;
    }
    const QString basePath = getDataPath();
    const std::string imagePath = (basePath + QChar('/') + QString::fromStdString(fileName)).toStdString();
    if (!QFileInfo::exists(QString::fromStdString(imagePath))) {
        LOG_WARN("Image file not found: {}", imagePath);
        return;
    }
    const std::string showId = QFileInfo(QString::fromStdString(fileName)).baseName().toStdString();
    UpdatePic(showId, imagePath);
}

void PicMatchWidget::UpdatePic(const std::string& showid, const std::string& imagePath)
{
    LOG_DEBUG("Run PicPlayer size: {} x {}", m_playerWidget->width(), m_playerWidget->height());
    std::unique_ptr<PicShowInfo> demodata = std::make_unique<PicShowInfo>();
    std::unique_ptr<FaceDetectionResult> faceResult = std::make_unique<FaceDetectionResult>();

    std::strncpy(demodata->imageId, showid.c_str(), IMAGE_ID_LEN - 1);
    demodata->imageId[IMAGE_ID_LEN - 1] = '\0';
    demodata->picReadTime = 1;
    std::strncpy(faceResult->imageId, showid.c_str(), IMAGE_ID_LEN - 1);
    faceResult->imageId[IMAGE_ID_LEN - 1] = '\0';
    
    if (!LoadJpegToRGBA(imagePath.c_str(), demodata.get())) {
        LOG_ERROR("Failed to load image: {}", imagePath);
        return;
    }
    LOG_DEBUG("InitPicPlayer size: {} x {}", m_playerWidget->width(), m_playerWidget->height());
    if (!PicPlayer_InputPicData(m_handle, 1, (void*)demodata.get())) {
        LOG_ERROR("Failed to input picture data to player");
        return;
    }

    if (DetectFacesInRgba(demodata.get(), faceResult.get()) != 0) {
        LOG_ERROR("Face detection failed");
        return;
    }
    
    if (faceResult->faces != nullptr && faceResult->faceCount > 0) {
        for (int i = 0; i < faceResult->faceCount; ++i) {
            faceResult->faces[i].x = 1.0f - faceResult->faces[i].x - faceResult->faces[i].width;
        }
    }
    
    if (!PicPlayer_InputFaceRecogResult(m_handle, (void*)faceResult.get())) {
        LOG_ERROR("Failed to input face recognition result to player");
    }
    
    if (faceResult->faces != nullptr && faceResult->faceCount > 0) {
        for (int i = 0; i < faceResult->faceCount; ++i) {
            m_faceShowWidget->addFaceImages(faceResult->faces[i].faceImageData, faceResult->faces[i].faceImageLength, faceResult->faces[i].faceImageWidth, faceResult->faces[i].faceImageHeight);
        }
    }
}

std::string PicMatchWidget::GetNextImageName()
{
    const QString basePath = getDataPath();

    if (!m_initialized) {
        m_imageNames.clear();
        try {
            QDir directory(basePath);
            if (directory.exists()) {
                directory.setFilter(QDir::Files);
                directory.setNameFilters(QStringList() << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp");
                directory.setSorting(QDir::Name);
                const QFileInfoList fileList = directory.entryInfoList();
                for (const QFileInfo& fileInfo : fileList) {
                    m_imageNames.push_back(fileInfo.fileName().toStdString());
                }
                LOG_INFO("Initialized image list from \"{}\" with {} images", basePath.toStdString(), m_imageNames.size());
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Error initializing image list: {}", e.what());
        }
        m_initialized = true;
    }

    if (!m_imageNames.empty() && m_currentIndex < m_imageNames.size()) {
        std::string fileName = m_imageNames[m_currentIndex];
        m_currentIndex++;
        return fileName;
    }
    if (!m_imageNames.empty() && m_currentIndex >= m_imageNames.size()) {
        return "";
    }
    return "default_image";
}


void* PicMatchWidget::PicCallbackByPlayer(int handle, int iMsg, void* pData, void* pUser)
{
    if (nullptr != pData && nullptr != pUser) {
        PicMatchWidget* pThis = reinterpret_cast<PicMatchWidget*>(pUser);
        if (Callback_ShowPicId == iMsg) {
            std::string showid((const char*)pData);
            LOG_DEBUG("showid : {}", showid.data());
            LOG_DEBUG("Run PicPlayer size: {} x {}", pThis->m_playerWidget->width(), pThis->m_playerWidget->height());
            QMetaObject::invokeMethod(pThis, [pThis, showid]() {
                pThis->OnRun(showid);
            }, Qt::QueuedConnection);
        }
    }
    return nullptr;
}

bool PicMatchWidget::LoadJpegToRGBA(const char* imagePath, PicShowInfo *demodata)
{
    if (!imagePath || !demodata) {
        LOG_ERROR("Invalid input parameters.");
        return false;
    }

    int channels, picWidth, picHeight;
    unsigned char* imageData = stbi_load(imagePath, &picWidth, &picHeight, &channels, 4);
    if (!imageData) {
        LOG_ERROR("Failed to load image: {}, Reason: {}", imagePath, (stbi_failure_reason() ? stbi_failure_reason() : "Unknown error"));
        return false;
    }

   struct StbiDeleter {
        void operator()(unsigned char* ptr) { stbi_image_free(ptr); }
    };
    std::unique_ptr<unsigned char, StbiDeleter> imageDataPtr(imageData);

    unsigned char* rotatedData = nullptr;
    RotateImage90Degrees(imageDataPtr.get(), rotatedData, picWidth, picHeight, 4);

    // Use smart pointer to manage rotated data
    std::unique_ptr<unsigned char[]> rotatedDataPtr(rotatedData);
    demodata->imageRgbaLen = picWidth * picHeight * 4;
    demodata->picWidth = picHeight;
    demodata->picHeight = picWidth;

    // Use malloc instead of new to match the free() in PicShowInfo destructor
    demodata->imageRgbaData = static_cast<char*>(malloc(demodata->imageRgbaLen));
    if (!demodata->imageRgbaData) {
        LOG_ERROR("Failed to allocate memory for image data.");
        return false;
    }

   std::memcpy(demodata->imageRgbaData, rotatedData, demodata->imageRgbaLen);
   return true;
}

void PicMatchWidget::RotateImage90Degrees(unsigned char* imageData, unsigned char*& rotatedData, int width, int height, int channels) {
    int newWidth = height;
    int newHeight = width;

    // Use malloc to match the free() in PicShowInfo destructor
    rotatedData = static_cast<unsigned char*>(malloc(newWidth * newHeight * channels));
    if (!rotatedData) {
        LOG_ERROR("Failed to allocate memory for rotated image");
        return;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channels; ++c) {
                rotatedData[((newHeight - 1 - x) * newWidth + y) * channels + c] = imageData[(y * width + x) * channels + c];
            }
        }
    }
}

// ==================== Button Bar（顶部横条）+ 人脸区在下，垂直布局 ====================
// 图标从组件输出目录 resource/icons 加载（与 CMake POST_BUILD 复制路径一致），避免 DLL 内嵌 qrc 的链接问题

void PicMatchWidget::CreateButtonBar(QWidget* parent)
{
    if (!parent)
        return;

    const QString iconDir = componentIconsPath();

    m_buttonBarWidget = new QFrame(parent);
    m_buttonBarWidget->setObjectName("ButtonBarWidget");
    m_buttonBarWidget->setFixedHeight(38);
    m_buttonBarWidget->setFrameShape(QFrame::NoFrame);

    QHBoxLayout* barLayout = new QHBoxLayout(m_buttonBarWidget);
    barLayout->setContentsMargins(12, 0, 0, 0);
    barLayout->setSpacing(8);
    barLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_runButton = new QToolButton(m_buttonBarWidget);
    m_runButton->setObjectName("RunButton");
    m_runButton->setFixedSize(46, 38);
    m_runButton->setIcon(QIcon(iconDir + QStringLiteral("/start.svg")));
    m_runButton->setIconSize(QSize(16, 16));
    m_runButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_runButton->setToolTip(tr("启动"));
    connect(m_runButton, &QToolButton::clicked, this, &PicMatchWidget::OnRunButtonClicked);
    barLayout->addWidget(m_runButton);

    m_stopButton = new QToolButton(m_buttonBarWidget);
    m_stopButton->setObjectName("StopButton");
    m_stopButton->setFixedSize(46, 38);
    m_stopButton->setIcon(QIcon(iconDir + QStringLiteral("/stop.svg")));
    m_stopButton->setIconSize(QSize(16, 16));
    m_stopButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_stopButton->setToolTip(tr("停止"));
    connect(m_stopButton, &QToolButton::clicked, this, &PicMatchWidget::OnStopButtonClicked);
    barLayout->addWidget(m_stopButton);

    barLayout->addStretch();

    m_configButton = new QToolButton(m_buttonBarWidget);
    m_configButton->setObjectName("ConfigButton");
    m_configButton->setFixedSize(46, 38);
    m_configButton->setIcon(QIcon(iconDir + QStringLiteral("/settings.svg")));
    m_configButton->setIconSize(QSize(16, 16));
    m_configButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_configButton->setToolTip(tr("配置"));
    connect(m_configButton, &QToolButton::clicked, this, &PicMatchWidget::OnConfigButtonClicked);
    barLayout->addWidget(m_configButton);
}

// ==================== 配置面板（滑出页） ====================

void PicMatchWidget::CreateConfigPanel(QStackedWidget* stack)
{
    if (!stack)
        return;

    m_configPanel = new QFrame(this);
    m_configPanel->setObjectName("ConfigPanel");

    QVBoxLayout* configLayout = new QVBoxLayout(m_configPanel);
    configLayout->setContentsMargins(12, 12, 12, 12);
    configLayout->setSpacing(12);

    QLabel* pathLabel = new QLabel(tr("图片查看路径 (componentDataPath)："), m_configPanel);
    configLayout->addWidget(pathLabel);

    QHBoxLayout* pathRow = new QHBoxLayout();
    m_configPathEdit = new QLineEdit(m_configPanel);
    m_configPathEdit->setPlaceholderText(tr("留空则使用默认路径"));
    m_configPathEdit->setText(m_customDataPath);
    m_configPathEdit->setMinimumHeight(28);
    pathRow->addWidget(m_configPathEdit);

    QPushButton* browseBtn = new QPushButton(tr("浏览..."), m_configPanel);
    browseBtn->setObjectName("PrimaryButton");
    connect(browseBtn, &QPushButton::clicked, this, &PicMatchWidget::OnConfigBrowse);
    pathRow->addWidget(browseBtn);
    configLayout->addLayout(pathRow);

    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    QPushButton* applyBtn = new QPushButton(tr("确定"), m_configPanel);
    applyBtn->setObjectName("PrimaryButton");
    connect(applyBtn, &QPushButton::clicked, this, &PicMatchWidget::OnConfigApply);
    QPushButton* backBtn = new QPushButton(tr("返回"), m_configPanel);
    backBtn->setObjectName("SecondaryButton");
    connect(backBtn, &QPushButton::clicked, this, [this]() {
        if (m_rightStack)
            m_rightStack->setCurrentIndex(0);
    });
    btnRow->addWidget(applyBtn);
    btnRow->addWidget(backBtn);
    configLayout->addLayout(btnRow);
    configLayout->addStretch();

    stack->addWidget(m_configPanel);
}

void PicMatchWidget::updateButtonBarStyle()
{
    if (!m_buttonBarWidget)
        return;
    const QString titleBarBg = themeColor(m_lastThemeColors, "titleBarBackground", "#2d2d30");
    const QString titleBarBorder = themeColor(m_lastThemeColors, "titleBarBorder", "#3c3c3c");
    const QString buttonHover = themeColor(m_lastThemeColors, "buttonHover", "#3c3c3c");
    const QString textPrimary = themeColor(m_lastThemeColors, "textPrimary", "#cccccc");
    QString sheet = QStringLiteral("QFrame#ButtonBarWidget { background-color: %1; border: none; border-bottom: 1px solid %2; } "
                                  "QToolButton { border: none; background: transparent; color: %3; } "
                                  "QToolButton:hover { background-color: %4; } "
                                  "QToolButton#ConfigButton, QToolButton#ConfigButton:hover { color: %3; } ")
                        .arg(titleBarBg, titleBarBorder, textPrimary, buttonHover);
    if (m_running && m_runButton) {
        sheet += QStringLiteral("QToolButton#RunButton { background-color: #4CAF50; color: white; } "
                                "QToolButton#RunButton:hover, QToolButton#RunButton:disabled { background-color: #4CAF50; color: white; } ");
    }
    m_buttonBarWidget->setStyleSheet(sheet);
}

void PicMatchWidget::applyTheme(QVariantMap themeColors)
{
    m_lastThemeColors = themeColors;
    const QString contentBg = themeColor(themeColors, "contentBackground", "#1e1e1e");
    const QString textPrimary = themeColor(themeColors, "textPrimary", "#cccccc");
    const QString borderColor = themeColor(themeColors, "border", "#3c3c3c");
    const QString buttonHover = themeColor(themeColors, "buttonHover", "#3c3c3c");
    const QString appTileBorder = themeColor(themeColors, "appTileBorder", "#007acc");

    if (m_rightPanel) {
        m_rightPanel->setStyleSheet(QStringLiteral("QWidget#RightPanel { background-color: %1; }").arg(contentBg));
    }
    updateButtonBarStyle();
    if (m_faceShowWidget) {
        const QString faceShowSheet =
            QStringLiteral("QLabel#FaceShowSectionHeader { color: %1; font-size: 13px; font-family: 'Segoe UI', 'Microsoft YaHei UI', sans-serif; padding: 8px 0 4px 0; } "
                           "QScrollArea#FaceShowScrollArea { background-color: %2; border: none; } "
                           "QWidget#FaceShowContainer { background-color: %2; } ")
                .arg(textPrimary, contentBg);
        m_faceShowWidget->setStyleSheet(faceShowSheet);
    }
    if (m_configPanel) {
        const QString configSheet =
            QStringLiteral("QFrame#ConfigPanel { background-color: %1; } "
                           "QLabel { color: %2; font-size: 12px; } "
                           "QLineEdit { background-color: %3; color: %2; border: 1px solid %3; padding: 6px; } "
                           "QPushButton#PrimaryButton { background: %4; color: white; border: none; padding: 6px 16px; } "
                           "QPushButton#PrimaryButton:hover { background: %4; } "
                           "QPushButton#SecondaryButton { background: %3; color: %2; border: 1px solid %3; padding: 6px 16px; } "
                           "QPushButton#SecondaryButton:hover { background: %5; }")
                .arg(contentBg, textPrimary, borderColor, appTileBorder, buttonHover);
        m_configPanel->setStyleSheet(configSheet);
    }
}

void PicMatchWidget::loadDataPath()
{
    QSettings s(kSettingsOrg, kSettingsApp);
    m_customDataPath = s.value(QString::fromUtf8(kSettingsDataPathKey)).toString().trimmed();
}

void PicMatchWidget::saveDataPath()
{
    QString path = m_configPathEdit ? m_configPathEdit->text().trimmed() : m_customDataPath;
    QSettings s(kSettingsOrg, kSettingsApp);
    s.setValue(QString::fromUtf8(kSettingsDataPathKey), path);
    m_customDataPath = path;
    m_initialized = false;
    m_imageNames.clear();
    m_currentIndex = 0;
}

void PicMatchWidget::OnConfigButtonClicked()
{
    if (m_rightStack) {
        if (m_configPathEdit)
            m_configPathEdit->setText(m_customDataPath);
        m_rightStack->setCurrentIndex(1);
    }
}

void PicMatchWidget::OnConfigApply()
{
    saveDataPath();
    if (m_rightStack)
        m_rightStack->setCurrentIndex(0);
}

void PicMatchWidget::OnConfigBrowse()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择图片查看路径"), m_configPathEdit ? m_configPathEdit->text() : getDataPath());
    if (!dir.isEmpty() && m_configPathEdit)
        m_configPathEdit->setText(QDir(dir).absolutePath());
}

void PicMatchWidget::UpdateUIState()
{
    if (!m_runButton || !m_stopButton) {
        return;
    }

    m_runButton->setEnabled(!m_running);
    m_stopButton->setEnabled(m_running);
    updateButtonBarStyle();
}

// ==================== Button Click Handlers ====================

void PicMatchWidget::OnRunButtonClicked()
{
    LOG_INFO("Run button clicked");
    if (!m_running) {
        Run();
        UpdateUIState();
    }
}

void PicMatchWidget::OnStopButtonClicked()
{
    LOG_INFO("Stop button clicked");
    if (m_running) {
        Quit();
        UpdateUIState();
    }
}
