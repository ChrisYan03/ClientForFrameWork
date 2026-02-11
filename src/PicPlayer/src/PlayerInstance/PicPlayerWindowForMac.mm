#include "PicPlayerWindowForMac.h"
#include "glfw3.h"
#define GLFW_EXPOSE_NATIVE_COCOA
#include "glfw3native.h"
#import <Cocoa/Cocoa.h>
#include <iostream>

bool GetWindowSizeForMac(void* hwnd, int& width, int& height)
{
    NSView* windowView = (__bridge NSView *)hwnd;
    if (!windowView)
        return false;

    NSRect frame = [windowView frame];
    width = frame.size.width;
    height = frame.size.height;
    return true;
}

bool SetChildWindow(void* parentWnd, void* childWnd)
{
    // 获取 NSView 对象（通过qwidget获取的句柄是NSView而非NSWindow）
    NSView* nsParentView = (__bridge NSView *)parentWnd;
    if (!nsParentView) {
        std::cerr << "Failed to get parent nsView" << std::endl;
        return false;
    }

    // 获取glfw窗口的NSWindow
    GLFWwindow* glfwChildWindow = static_cast<GLFWwindow*>(childWnd);
    id childWindow = glfwGetCocoaWindow(glfwChildWindow);
    if (!childWindow) {
        std::cerr << "Failed to get chlid CocoaWindow" << std::endl;
        return false;
    }
    NSWindow* nsChildWindow = (__bridge NSWindow *)childWindow;
    if (!nsChildWindow) {
        std::cerr << "Failed to get chlid NSWindow" << std::endl;
        return false;
    }
    // 获取子窗口的内容视图
    NSView* childContentView = [nsChildWindow contentView];
    if (!childContentView) {
        std::cerr << "Failed to get child content view" << std::endl;
        return false;
    }
    // 设置子窗口的属性
    [nsChildWindow setLevel:NSNormalWindowLevel];
    [nsChildWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [nsChildWindow setOpaque:NO];
    [nsChildWindow setHasShadow:NO];
    [nsChildWindow setMovableByWindowBackground:YES];
    [nsChildWindow setMovable:YES];
    [nsChildWindow setTitle:@""];
    // 隐藏子窗口的窗口装饰
    [nsChildWindow setStyleMask:NSWindowStyleMaskBorderless];
    // 隐藏子窗口本身
    [nsChildWindow orderOut:(nil)];

    // 设置子窗口的尺寸和位置
    NSRect parentFrame = [nsParentView frame];
    [nsChildWindow setFrame:parentFrame display:YES];
    // 将子窗口的内容视图添加到父窗口的内容视图中
    [nsParentView addSubview:childContentView];

    return true;
}
