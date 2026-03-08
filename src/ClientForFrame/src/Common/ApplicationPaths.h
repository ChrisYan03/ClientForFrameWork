#ifndef APPLICATIONPATHS_H
#define APPLICATIONPATHS_H

#include <QString>

/**
 * @brief 应用运行时路径：提供 config、Component 等查找根目录。
 * macOS .app 下为 .app 所在目录；否则为可执行文件所在目录。
 * 若在 macOS 上从 Release 子目录找到 config，可调用 setResolvedBaseDir 缓存，后续 baseDir() 返回该路径。
 */
class ApplicationPaths
{
public:
    ApplicationPaths() = default;

    /** 初始根目录（未解析 Release 时）：exe 所在目录，或 macOS .app 的父目录 */
    QString initialBaseDir() const;

    /** 用于查找 config、Component 的根目录；若已 setResolvedBaseDir 则返回缓存，否则返回 initialBaseDir() */
    QString baseDir() const;

    /** 在 macOS 上从 Release 子目录找到 config 时调用，后续 baseDir() 将返回该路径 */
    void setResolvedBaseDir(const QString &dir) { m_resolvedBaseDir = dir; }

private:
    QString m_resolvedBaseDir;
};

#endif // APPLICATIONPATHS_H
