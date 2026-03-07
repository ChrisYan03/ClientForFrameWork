#include "ResourcePathManager.h"
#include "LogUtil.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QtGlobal>
#include <algorithm>

namespace ClientForFrame {

// Static member initialization
std::string ResourcePathManager::s_applicationBasePath;
bool ResourcePathManager::s_initialized = false;

// ==============================================================================
// Resource Path Manager Implementation
// ==============================================================================
void ResourcePathManager::initialize() {
    if (!s_initialized) {
        s_applicationBasePath = getQtApplicationDirPath();
        s_initialized = true;
        LOG_INFO("ResourcePathManager initialized with base path: {}", s_applicationBasePath);
    }
}

std::string ResourcePathManager::getApplicationBasePath() {
    initialize();
    return s_applicationBasePath;
}

std::string ResourcePathManager::getComponentDataPath(const std::string& componentName) {
    std::string exeDir = getApplicationBasePath();
    std::string componentBin = exeDir + "/Component/" + componentName + "/bin";

    // Check if component bin directory exists
    if (fileExists(componentBin)) {
        return componentBin;
    }

    // Fallback to exe directory
    return exeDir;
}

std::string ResourcePathManager::getImageBasePath() {
    return getComponentDataPath();
}

std::string ResourcePathManager::getConfigPath() {
    std::string basePath = getApplicationBasePath();
    std::vector<std::string> configPaths = {
        basePath + "/config",
        basePath + "/../config"
    };

    for (const auto& path : configPaths) {
        if (fileExists(path)) {
            return path;
        }
    }

    // Default to config directory in exe path
    return basePath + "/config";
}

std::string ResourcePathManager::getLogPath() {
    std::string basePath = getApplicationBasePath();
    std::string logPath = basePath + "/logs";
    ensureDirectoryExists(logPath);
    return logPath;
}

std::string ResourcePathManager::getDataPath() {
    std::string basePath = getApplicationBasePath();
#if defined(Q_OS_MAC)
    // macOS 默认使用工程下的 picdata 目录
    static const std::string macDefaultPicdata("/Users/chrisyan/ClientForFrameWork/picdata");
    std::vector<std::string> dataPaths = {
        macDefaultPicdata,
        getComponentDataPath() + "/picdata",
        basePath + "/picdata",
        basePath + "/../picdata"
    };
#else
    std::vector<std::string> dataPaths = {
        getComponentDataPath() + "/picdata",
        basePath + "/picdata",
        basePath + "/../picdata"
    };
#endif

    for (const auto& path : dataPaths) {
        if (fileExists(path)) {
            return path;
        }
    }

#if defined(Q_OS_MAC)
    std::string defaultPath = macDefaultPicdata;
#else
    std::string defaultPath = basePath + "/picdata";
#endif
    ensureDirectoryExists(defaultPath);
    return defaultPath;
}

std::string ResourcePathManager::getComponentPath(const std::string& componentId) {
    std::string basePath = getApplicationBasePath();
    return basePath + "/Component/" + componentId;
}

std::string ResourcePathManager::getExternalLibPath() {
    // Check for environment variable first
    if (const char* extDir = std::getenv("EXT_DIR")) {
        return std::string(extDir);
    }

    // Default relative path
    return getApplicationBasePath() + "/../ext";
}

std::string ResourcePathManager::resolveFilePath(const std::string& filename,
                                                  const std::vector<std::string>& searchPaths) {
    for (const auto& basePath : searchPaths) {
        std::string fullPath = basePath + "/" + filename;
        if (fileExists(fullPath)) {
            return fullPath;
        }
    }

    // If not found, return the first path as default
    if (!searchPaths.empty()) {
        return searchPaths[0] + "/" + filename;
    }

    return filename;
}

bool ResourcePathManager::fileExists(const std::string& filePath) {
    QFileInfo fileInfo(QString::fromStdString(filePath));
    return fileInfo.exists();
}

bool ResourcePathManager::ensureDirectoryExists(const std::string& dirPath) {
    QDir dir(QString::fromStdString(dirPath));
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}

std::string ResourcePathManager::toNativePath(const std::string& path) {
    return QDir::toNativeSeparators(QString::fromStdString(path)).toStdString();
}

std::string ResourcePathManager::joinPath(const std::vector<std::string>& components) {
    if (components.empty()) {
        return "";
    }

    QDir result(QString::fromStdString(components[0]));
    for (size_t i = 1; i < components.size(); ++i) {
        result = result.filePath(QString::fromStdString(components[i]));
    }

    return result.path().toStdString();
}

std::string ResourcePathManager::getQtApplicationDirPath() {
    return QDir(QCoreApplication::applicationDirPath()).absolutePath().toStdString();
}

// ==============================================================================
// Path Builder Implementation
// ==============================================================================
PathBuilder& PathBuilder::append(const std::string& component) {
    m_components.push_back(component);
    return *this;
}

PathBuilder& PathBuilder::append(const std::vector<std::string>& components) {
    m_components.insert(m_components.end(), components.begin(), components.end());
    return *this;
}

std::string PathBuilder::build() const {
    return ResourcePathManager::joinPath(m_components);
}

void PathBuilder::clear() {
    m_components.clear();
}

// ==============================================================================
// Convenience Functions Implementation
// ==============================================================================
std::string getSampleImagePath(const std::string& imageName) {
    std::vector<std::string> searchPaths = {
        ResourcePathManager::getComponentDataPath(),
        ResourcePathManager::getApplicationBasePath(),
        ResourcePathManager::getApplicationBasePath() + "/.."
    };

    return ResourcePathManager::resolveFilePath(imageName, searchPaths);
}

std::string getDataFilePath(const std::string& filename) {
    std::string dataPath = ResourcePathManager::getDataPath();
    return ResourcePathManager::joinPath({dataPath, filename});
}

std::string getConfigFilePath(const std::string& filename) {
    std::string configPath = ResourcePathManager::getConfigPath();
    return ResourcePathManager::joinPath({configPath, filename});
}

std::string getLogFilePath(const std::string& filename) {
    std::string logPath = ResourcePathManager::getLogPath();
    return ResourcePathManager::joinPath({logPath, filename});
}

} // namespace ClientForFrame