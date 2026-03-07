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
    width = static_cast<int>(frame.size.width);
    height = static_cast<int>(frame.size.height);
    return true;
}

// 将 GLFW 的 contentView 嵌入到父 view，嵌入后给 GLFW 窗口设一个空的 contentView，
// 避免 AppKit 对已移走的 view 调用 isOpaque 等导致崩溃（NSIndexPath isOpaque）
bool SetChildWindow(void* parentWnd, void* childWnd)
{
    NSView* nsParentView = (__bridge NSView *)parentWnd;
    if (!nsParentView) {
        std::cerr << "Failed to get parent nsView" << std::endl;
        return false;
    }

    GLFWwindow* glfwChildWindow = static_cast<GLFWwindow*>(childWnd);
    id childWindow = glfwGetCocoaWindow(glfwChildWindow);
    if (!childWindow) {
        std::cerr << "Failed to get child CocoaWindow" << std::endl;
        return false;
    }
    NSWindow* nsChildWindow = (__bridge NSWindow *)childWindow;
    if (!nsChildWindow) {
        std::cerr << "Failed to get child NSWindow" << std::endl;
        return false;
    }

    NSView* childContentView = [nsChildWindow contentView];
    if (!childContentView) {
        std::cerr << "Failed to get child content view" << std::endl;
        return false;
    }

    [nsChildWindow setLevel:NSNormalWindowLevel];
    [nsChildWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [nsChildWindow setOpaque:NO];
    [nsChildWindow setHasShadow:NO];
    [nsChildWindow setMovableByWindowBackground:YES];
    [nsChildWindow setMovable:YES];
    [nsChildWindow setTitle:@""];
    [nsChildWindow setStyleMask:NSWindowStyleMaskBorderless];
    [nsChildWindow orderOut:nil];

    NSRect parentFrame = [nsParentView bounds];
    [nsChildWindow setFrame:parentFrame display:YES];

    [nsParentView addSubview:childContentView];
    [childContentView setFrame:[nsParentView bounds]];
    [childContentView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

    // 关键：给 GLFW 窗口换一个空的 contentView，避免系统对已移走的 view 发 isOpaque 等导致崩溃
    NSView* placeholderView = [[NSView alloc] init];
    [nsChildWindow setContentView:placeholderView];

    return true;
}

void UpdateChildWindowFrameForMac(void* parentWnd, void* childWnd)
{
    (void)parentWnd;
    (void)childWnd;
    // contentView 已嵌入父 view 且设置了 autoresizingMask，无需再更新
}

void RemoveChildWindowForMac(void* parentWnd, void* childWnd)
{
    (void)parentWnd;
    (void)childWnd;
    // 未使用子窗口关系，销毁时由 GLFW 自行释放
}
