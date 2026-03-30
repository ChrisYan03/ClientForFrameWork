/**
 * @file ComponentLoaderFactory.cpp
 * @brief 组件加载器工厂实现
 */
#include "ComponentLoaderFactory.h"
#include "Loaders/dll/NativeDllComponentLoader.h"
#include "Loaders/process/StandaloneExeComponentLoader.h"
#include "Loaders/web/WebUrlComponentLoader.h"
#include "Loaders/process/EmbeddedExeComponentLoader.h"

ComponentLoaderFactory::ComponentLoaderFactory(QObject* parent)
    : QObject(parent)
{
    initializeBuiltInLoaders();
}

ComponentLoaderFactory::~ComponentLoaderFactory()
{
    // 清理所有加载器
    qDeleteAll(m_loaders);
    m_loaders.clear();
}

void ComponentLoaderFactory::initializeBuiltInLoaders()
{
    // 创建内置加载器
    m_loaders[ComponentType_NativeDll] = new NativeDllComponentLoader(this);
    m_loaders[ComponentType_StandaloneExe] = new StandaloneExeComponentLoader(this);
    m_loaders[ComponentType_WebUrl] = new WebUrlComponentLoader(this);
    m_loaders[ComponentType_EmbeddedExe] = new EmbeddedExeComponentLoader(this);
}

IComponentLoader* ComponentLoaderFactory::getLoader(ComponentType type)
{
    return m_loaders.value(type, nullptr);
}

bool ComponentLoaderFactory::registerLoader(ComponentType type, IComponentLoader* loader)
{
    if (!loader) {
        return false;
    }

    // 如果该类型已有加载器，先删除旧的
    if (m_loaders.contains(type)) {
        delete m_loaders[type];
    }

    m_loaders[type] = loader;
    return true;
}

bool ComponentLoaderFactory::isTypeSupported(ComponentType type) const
{
    return m_loaders.contains(type);
}

QList<ComponentType> ComponentLoaderFactory::supportedTypes() const
{
    return m_loaders.keys();
}