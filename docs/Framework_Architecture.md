# Framework 架构设计文档

## 概述

Framework 是一个轻量级、高性能的组件化框架，采用 **C++ 接口 + C 风格 API** 双接口设计。组件信息通过 `ComponentManifest` 结构体获取，组件只需实现 4 个核心虚函数即可接入框架。

**版本**: 2.1.0 | **更新**: 2025-03-16 | **理念**: 极简接口 + Manifest驱动

## 核心设计原则

1. **Manifest 驱动** - 组件信息（id/name/version/icon/qmlPage等）全部从 manifest.json 解析
2. **极简接口** - IComponent 只定义 4 个核心虚函数
3. **双向信息流** - Framework 加载 manifest 后传递给组件
4. **中间件模式** - ComponentService 作为统一入口

## 目录结构

```
src/Framework/
├── Interface/
│   ├── IComponentData.h          # ComponentManifest 数据结构
│   ├── IComponent.h               # 组件接口（4个虚函数）
│   ├── IComponentApi.h            # C 风格 API
│   └── ComponentService.h         # 统一服务入口
└── src/
    ├── ComponentManager.*         # 组件管理器
    ├── IComponentApiImpl.cpp     # C API 实现
    ├── ComponentManifest.cpp     # JSON 解析
    └── ComponentService.cpp      # 服务入口实现
```

## 架构图

```
┌─────────────────────────────────────────────────────────────┐
│                    主框架 (ClientForFrame)                   │
│  ComponentService::initialize(&engine, &appController)      │
└────────────────────────────┬────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────┐
│                    Framework 中间件                          │
│  1. 读取 components.json                                    │
│  2. 加载动态库 → createComponent()                         │
│  3. 解析 manifest.json → setManifest() 传递给组件           │
│  4. 调用 initialize() / registerQmlTypes()                │
└────────────────────────────┬────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────┐
│                    组件 (PicMatchComponent)                  │
│  - 继承 IComponent + QObject                               │
│  - m_manifest 成员接收 Framework 传递的信息                  │
│  - 实现 4 个核心虚函数                                      │
└─────────────────────────────────────────────────────────────┘
```

## IComponent 接口（仅4个虚函数）

```cpp
class IComponent {
public:
    virtual ~IComponent() = default;

    // 生命周期
    virtual bool initialize(QQmlEngine* engine, const QString& basePath) = 0;
    virtual void shutdown() = 0;

    // QML 集成
    virtual void registerQmlTypes(QQmlEngine* engine) = 0;

    // 组件通信
    virtual QObject* getInterface(const QString& name) = 0;
};
```

## 组件实现示例

```cpp
class PicMatchComponent : public QObject, public IComponent {
    Q_OBJECT

public:
    // 4个核心虚函数
    bool initialize(QQmlEngine* engine, const QString& basePath) override;
    void shutdown() override;
    void registerQmlTypes(QQmlEngine* engine) override;
    QObject* getInterface(const QString& name) override;

    // Framework 调用：接收 manifest 信息
    Q_INVOKABLE void setManifest(const ComponentManifest& manifest) {
        m_manifest = manifest;
    }

    // 组件信息通过 m_manifest 访问
    ComponentManifest m_manifest;
};

// 导出工厂函数
extern "C" PICMATCHCOMPONENT_API void* createComponent() {
    return new PicMatchComponent();
}
```

## ComponentService 使用

```cpp
#include <Framework/ComponentService.h>

int main() {
    ComponentService service;
    service.setBasePath(baseDir);
    service.initialize(&engine, &appController);
    // ...
    service.shutdown();
}
```

## 组件目录结构

```
Component/picmatch/
├── meta_info/
│   ├── manifest.json        # 组件元信息
│   └── icon.svg            # 图标
└── bin/
    ├── libcomponent.dylib  # 组件动态库
    └── qml/               # QML 模块
```

## manifest.json

```json
{
  "id": "picmatch",
  "name": "图像人脸识别",
  "version": "1.0.0",
  "description": "图像人脸识别组件",
  "author": "ClientForFrameWork",
  "type": "native",
  "icon": "icon.svg",
  "qmlPage": "PicMatchPage.qml"
}
```

## 生命周期

```
Framework                           组件
   │                                 │
   │  createComponent()              │
   │◄────────────────────────────────┤
   │                                 │
   │  load manifest.json             │
   │                                 │
   │  setManifest(manifest)         │
   │────────────────────────────────►│
   │                                 │
   │  initialize(engine, basePath)   │
   │────────────────────────────────►│
   │                                 │
   │  registerQmlTypes(engine)      │
   │────────────────────────────────►│
   │                                 │
   │  (应用运行)                     │
   │                                 │
   │  shutdown()                    │
   │────────────────────────────────►│
   │                                 │
```

## 技术栈

- **语言**: C++17
- **框架**: Qt6
- **JSON**: xpack
- **构建**: CMake 3.16+

---

**状态**: 架构 2.1 完成 | **文档**: v2.1.0
