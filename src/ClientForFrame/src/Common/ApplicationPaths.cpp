#include "ApplicationPaths.h"
#include <QCoreApplication>
#include <QDir>

QString ApplicationPaths::initialBaseDir() const
{
    QString exeDir = QDir(QCoreApplication::applicationDirPath()).absolutePath();
#if defined(Q_OS_MAC)
    if (exeDir.contains(QLatin1String(".app/Contents/MacOS"))) {
        QDir dir(exeDir);
        if (dir.cdUp() && dir.cdUp() && dir.cdUp())  // MacOS -> Contents -> .app -> 父目录
            return dir.absolutePath();
    }
#endif
    return exeDir;
}

QString ApplicationPaths::baseDir() const
{
    if (!m_resolvedBaseDir.isEmpty())
        return m_resolvedBaseDir;
    return initialBaseDir();
}
