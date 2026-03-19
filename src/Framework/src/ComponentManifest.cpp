/**
 * @file ComponentManifest.cpp
 * @brief 组件元数据结构体实现
 *
 * 此文件包含 xpack 相关代码，不应在 MOC 处理的头文件中包含。
 */
#include "../include/ComponentTypes.h"
#include "xpack/json.h"

// ==================== 辅助函数实现 ====================

bool loadComponentManifest(const QString &manifestPath, ComponentManifest &manifest) {
    try {
        // 手动使用 xpack 解析 JSON，避免在头文件中使用 XPACK_O 宏
        xpack::JsonData json;
        xpack::json::decode_file(manifestPath.toStdString(), json);

        if (!json) {
            return false;
        }

        // 读取各个字段
        auto getString = [&json](const char* key, const QString& defaultVal = QString()) -> QString {
            xpack::JsonData val = json[key];
            if (!val) return defaultVal;
            std::string result;
            if (val.Get(result)) {
                return QString::fromStdString(result);
            }
            return defaultVal;
        };

        auto getStringList = [&json](const char* key) -> QStringList {
            QStringList result;
            xpack::JsonData arr = json[key];
            if (!arr || !arr.IsArray()) return result;

            size_t size = arr.Size();
            for (size_t i = 0; i < size; ++i) {
                xpack::JsonData item = arr[i];
                if (item) {
                    std::string strVal;
                    if (item.Get(strVal)) {
                        result.append(QString::fromStdString(strVal));
                    }
                }
            }
            return result;
        };

        manifest.id = getString("id", "picmatch").toStdString();
        manifest.name = getString("name", "Unknown").toStdString();
        manifest.version = getString("version", "1.0.0").toStdString();
        manifest.description = getString("description", "").toStdString();
        manifest.author = getString("author", "").toStdString();
        manifest.type = getString("type", "native").toStdString();
        manifest.icon = getString("icon", "").toStdString();
        manifest.qmlPage = getString("qmlPage", "").toStdString();
        manifest.dataPath = getString("dataPath", "").toStdString();
        manifest.module = getString("module", "").toStdString();
        // 转换 QStringList 为 std::vector<std::string>
        QStringList depsList = getStringList("dependencies");
        manifest.dependencies.clear();
        for (const auto &dep : depsList) {
            manifest.dependencies.push_back(dep.toStdString());
        }

        return true;
    } catch (const std::exception &e) {
        return false;
    }
}

QVariantMap manifestToVariantMap(const ComponentManifest &manifest) {
    QVariantMap map;
    map["id"] = QString::fromStdString(manifest.id);
    map["name"] = QString::fromStdString(manifest.name);
    map["version"] = QString::fromStdString(manifest.version);
    map["description"] = QString::fromStdString(manifest.description);
    map["author"] = QString::fromStdString(manifest.author);
    map["type"] = QString::fromStdString(manifest.type);
    map["icon"] = QString::fromStdString(manifest.icon);
    map["qmlPage"] = QString::fromStdString(manifest.qmlPage);
    map["dataPath"] = QString::fromStdString(manifest.dataPath);

    QStringList deps;
    for (const auto &dep : manifest.dependencies) {
        deps.append(QString::fromStdString(dep));
    }
    map["dependencies"] = deps;

    return map;
}
