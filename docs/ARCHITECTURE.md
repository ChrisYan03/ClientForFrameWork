# ClientForFrameWork Architecture

## Table of Contents

1. [Overview](#overview)
2. [Architectural Principles](#architectural-principles)
3. [Component Architecture](#component-architecture)
4. [Memory Management](#memory-management)
5. [Threading Model](#threading-model)
6. [Build System](#build-system)
7. [Data Flow](#data-flow)
8. [Error Handling](#error-handling)
9. [Future Improvements](#future-improvements)

## Overview

ClientForFrameWork is a cross-platform image processing framework built with Qt/C++ and a plugin-based architecture. The system is designed to be modular, extensible, and maintainable with strong emphasis on memory safety and performance.

### Key Technologies

- **Qt 6**: UI framework and cross-platform abstraction
- **OpenCV**: Computer vision and image processing
- **CMake**: Cross-platform build system
- **C++17**: Modern C++ features for safety and performance
- **OpenGL**: Hardware-accelerated rendering

### System Requirements

- **Windows**: MSVC 2019+, Windows 10+
- **macOS**: Xcode 12+, macOS 10.15+

## Architectural Principles

### 1. Separation of Concerns

Each component has a well-defined responsibility:
- **PicPlayer**: Image display and rendering
- **PicRecognition**: Face detection and recognition
- **ClientForFrame**: Main application and component management

### 2. Interface-Based Design

Components communicate through well-defined interfaces:
```cpp
namespace ComponentInterface {
    struct PicShowInfo { /* ... */ };
    struct FaceDetectionResult { /* ... */ };
    using PlayerMsgCallback = void*(*)(int, int, void*, void*);
}
```

### 3. RAII and Memory Safety

All resources are managed using RAII principles:
```cpp
auto imageData = ClientForFrame::StbImagePtr(stbi_load(...));
auto faceData = ClientForFrame::make_array<char>(size);
```

### 4. Plugin Architecture

Components are loaded dynamically:
```cpp
using ComponentRegisterFunc = void (*)(QQmlEngine*, QObject*);
auto reg = reinterpret_cast<ComponentRegisterFunc>(lib.resolve(symbol));
```

## Component Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────┐
│                   ClientForFrame                        │
│                   (Main Application)                    │
│  ┌─────────────────────────────────────────────────┐   │
│  │              AppController                      │   │
│  │  - Component Management                        │   │
│  │  - QML Integration                             │   │
│  │  - Theme Management                            │   │
│  └─────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
                          │
                          │ Plugin Interface
                          │
        ┌─────────────────┼─────────────────┐
        │                 │                 │
┌───────▼────────┐ ┌──────▼───────┐ ┌──────▼───────┐
│  PicMatch      │ │   Other      │ │   Future     │
│  Component     │ │  Components  │ │  Components  │
│  ┌──────────┐  │ │              │ │              │
│  │PicPlayer │  │ │              │ │              │
│  │  - Img  │  │ │              │ │              │
│  │  - GLFW │  │ │              │ │              │
│  │  - OpenGL│ │ │              │ │              │
│  └──────────┘  │ │              │ │              │
│  ┌──────────┐  │ │              │ │              │
│  │PicRecog  │  │ │              │ │              │
│  │  - OpenCV│  │ │              │ │              │
│  │  - DNN   │  │ │              │ │              │
│  └──────────┘  │ │              │ │              │
└────────────────┘ └──────────────┘ └──────────────┘
```

### Component Lifecycle

1. **Discovery**: Read `config/components.json`
2. **Loading**: Load component DLL/shared library
3. **Registration**: Component registers its functionality
4. **Initialization**: Component-specific setup
5. **Operation**: Normal operation with callbacks
6. **Shutdown**: Graceful cleanup and resource release

### Component Communication

Components communicate through:
- **C-Style API**: For cross-language compatibility
- **Qt Signals/Slots**: For UI integration
- **Callback Functions**: For asynchronous events

## Memory Management

### Smart Pointer Strategy

```cpp
// STB Image Management
using StbImagePtr = ManagedResource<unsigned char, StbImageDeleter>;

// Array Management
template<typename T>
using ArrayPtr = ManagedResource<T, ArrayDeleter<T>>;

// QLibrary Management
static std::unique_ptr<QLibrary> s_shutdownLib;
```

### Resource Ownership

1. **Clear Ownership**: Each resource has a defined owner
2. **Transfer Semantics**: Move semantics for ownership transfer
3. **No Leaks**: RAII ensures automatic cleanup
4. **Exception Safety**: Stack unwinding cleans up resources

### Memory Flow Example

```
Image Load → STB Load (malloc) → Smart Pointer Wrap →
Component Use → Automatic Free (when out of scope)
```

### Critical Memory Rules

1. **Never mix new/delete with malloc/free**
2. **Always use smart pointers for dynamic allocation**
3. **Use move semantics when transferring ownership**
4. **Let Qt manage widget parent-child relationships**

## Threading Model

### Thread Architecture

```
Main Thread (Qt GUI)
├── QML Engine
├── UI Rendering
└── Event Loop

Worker Threads
├── Image Loading (PicPlayer)
├── Face Detection (PicRecognition)
└── Background Processing

Communication
├── Qt Signals/Slots (thread-safe)
├── Callbacks (with mutex protection)
└── Shared Data (with synchronization)
```

### Thread Safety Mechanisms

1. **HandleRegister**: Thread-safe object registration
   ```cpp
   int RegisterObjInstance(std::unique_ptr<T>&& obj_ptr) {
       std::lock_guard<std::mutex> guard(m_mutex);
       // ... thread-safe registration
   }
   ```

2. **Qt Signals/Slots**: Automatic thread marshaling
   ```cpp
   QMetaObject::invokeMethod(this, [this, showid]() {
       this->OnRun(showid);
   }, Qt::QueuedConnection);
   ```

3. **Mutex Protection**: For shared data access

## Build System

### CMake Architecture

```
ClientForFrameWork/
├── CMakeLists.txt (Root)
│   ├── cmake/PlatformConfig.cmake
│   └── cmake/FindThirdParty.cmake
├── src/CMakeLists.txt (Main)
├── src/Component/*/CMakeLists.txt
└── src/ClientForFrame/CMakeLists.txt
```

### Build Process

1. **Platform Detection**: Determine OS and compiler
2. **Dependency Finding**: Locate Qt, OpenCV, etc.
3. **Component Compilation**: Build all components
4. **Linking**: Create executable and libraries
5. **Resource Copying**: Copy required files to output

### Cross-Platform Support

**Windows (MSVC)**:
```cmake
if(MSVC)
    add_compile_options(/W4)
    add_definitions(-DUNICODE -D_UNICODE)
endif()
```

**macOS (Clang)**:
```cmake
if(APPLE)
    add_compile_options(-Wall -Wextra)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()
```

## Data Flow

### Image Processing Pipeline

```
1. User Selection (QML)
   ↓
2. AppController::requestImage()
   ↓
3. PicMatchWidget::UpdatePic()
   ↓
4. LoadJpegToRGBA() [STB Image]
   ↓
5. RotateImage90Degrees() [CPU Transform]
   ↓
6. PicPlayer_InputPicData() [GPU Upload]
   ↓
7. DetectFacesInRgba() [OpenCV]
   ↓
8. PicPlayer_InputFaceRecogResult() [GPU Display]
   ↓
9. FaceShowWidget::addFaceImages() [UI Update]
```

### Component Communication Flow

```
Component A                    Component B
     │                              │
     │  1. Load Library             │
     ├─────────────────────────────>│
     │                              │
     │  2. Resolve Symbol           │
     ├─────────────────────────────>│
     │                              │
     │  3. Call Register Function   │
     ├─────────────────────────────>│
     │                              │
     │  4. Register Callback        │
     │<─────────────────────────────┤
     │                              │
     │  5. Runtime Communication    │
     ├─────────────────────────────>│
     │  (via Callbacks)             │
```

## Error Handling

### Error Handling Strategy

```cpp
try {
    THROW_IF_NULL(ptr, "Pointer cannot be null");
    // ... code ...
} catch (const ClientForFrame::Exception& e) {
    LOG_ERROR("Error: {}", e.what());
    // Handle error
}
```

### Error Categories

1. **InvalidArgument**: Bad input parameters
2. **OutOfMemory**: Allocation failure
3. **FileNotFound**: Missing resource files
4. **InitializationFailed**: Setup failure
5. **ResourceLoadFailed**: Component/plugin load failure

### Error Propagation

```
Component Error → Exception → Catch Block →
Log Error → Update UI → Graceful Degradation
```

## Future Improvements

### Short Term (1-3 months)

1. **Enhanced Error Recovery**
   - Component restart on failure
   - Graceful degradation
   - User-friendly error messages

2. **Performance Optimization**
   - Multi-threaded image processing
   - GPU acceleration for more operations
   - Caching for frequently used resources

3. **Testing Infrastructure**
   - Unit tests for core components
   - Integration tests for component interaction
   - Memory leak detection in CI/CD

### Medium Term (3-6 months)

1. **Additional Components**
   - Video processing component
   - Image editing component
   - Export/filter component

2. **Enhanced UI**
   - More sophisticated QML interface
   - Custom controls
   - Animation and transitions

3. **Configuration Management**
   - Runtime configuration changes
   - User preferences persistence
   - Configuration validation

### Long Term (6-12 months)

1. **Cloud Integration**
   - Cloud-based face recognition
   - Image storage and synchronization
   - Remote processing capabilities

2. **Machine Learning**
   - Advanced ML models
   - Real-time video analysis
   - Object detection beyond faces

3. **Mobile Support**
   - iOS and Android versions
   - Touch-optimized interface
   - Mobile-specific features

## Performance Considerations

### Current Performance Characteristics

- **Image Loading**: ~10-50ms per image (depending on size)
- **Face Detection**: ~50-200ms per image (depending on complexity)
- **UI Response**: <16ms (60 FPS target)
- **Memory Usage**: ~50-200MB (depending on workload)

### Optimization Strategies

1. **Lazy Loading**: Load resources on demand
2. **Caching**: Cache frequently used data
3. **Thread Pools**: Reuse threads for parallel work
4. **GPU Acceleration**: Offload work to GPU when possible
5. **Memory Pooling**: Reduce allocation overhead

## Security Considerations

### Current Security Measures

1. **DLL Validation**: Verify component integrity
2. **Path Sanitization**: Prevent directory traversal
3. **Resource Limits**: Prevent resource exhaustion
4. **Error Handling**: Prevent information disclosure

### Future Security Improvements

1. **Code Signing**: Sign components and executables
2. **Sandboxing**: Isolate component execution
3. **Encryption**: Encrypt sensitive data
4. **Audit Logging**: Track security-relevant events

---

**Document Version**: 1.0
**Last Updated**: 2026-03-06
**Maintained By**: Development Team