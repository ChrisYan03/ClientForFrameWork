#pragma once

#include <string>
#include <vector>
#include <QDir>
#include <QCoreApplication>

namespace ClientForFrame {

// ==============================================================================
// Path Manager Class
// ==============================================================================
class ResourcePathManager {
public:
    // Get application base directory
    static std::string getApplicationBasePath();

    // Get component data directory
    static std::string getComponentDataPath(const std::string& componentName = "PicMatch");

    // Get image base directory
    static std::string getImageBasePath();

    // Get configuration directory
    static std::string getConfigPath();

    // Get log directory
    static std::string getLogPath();

    // Get data directory (for images, samples, etc.)
    static std::string getDataPath();

    // Get plugin/component directory
    static std::string getComponentPath(const std::string& componentId);

    // Get external libraries directory
    static std::string getExternalLibPath();

    // Resolve file path with fallbacks
    // Tries primary path first, then fallback paths if not found
    static std::string resolveFilePath(const std::string& filename,
                                       const std::vector<std::string>& searchPaths);

    // Check if file exists
    static bool fileExists(const std::string& filePath);

    // Create directory if it doesn't exist
    static bool ensureDirectoryExists(const std::string& dirPath);

    // Convert to platform-specific path separators
    static std::string toNativePath(const std::string& path);

    // Join path components
    static std::string joinPath(const std::vector<std::string>& components);

private:
    // Cache for application base path
    static std::string s_applicationBasePath;
    static bool s_initialized;

    // Initialize the path manager
    static void initialize();

    // Get Qt application directory as string
    static std::string getQtApplicationDirPath();
};

// ==============================================================================
// Path Builder Utility
// ==============================================================================
class PathBuilder {
public:
    PathBuilder() = default;

    PathBuilder& append(const std::string& component);
    PathBuilder& append(const std::vector<std::string>& components);
    std::string build() const;
    void clear();

private:
    std::vector<std::string> m_components;
};

// ==============================================================================
// Convenience Functions
// ==============================================================================
// Get full path to a sample image
std::string getSampleImagePath(const std::string& imageName = "beauty_20250216152514.jpg");

// Get full path to a data file
std::string getDataFilePath(const std::string& filename);

// Get full path to a configuration file
std::string getConfigFilePath(const std::string& filename);

// Get full path to a log file
std::string getLogFilePath(const std::string& filename);

} // namespace ClientForFrame