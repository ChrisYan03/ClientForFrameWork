#include "MainWindowSetup.h"
#include <QBitmap>
#include <QImage>
#include <QPainter>
#include <QQuickWindow>
#include <QRegion>
#include <QTimer>
#include <QWindow>
#if defined(Q_OS_WIN)
#include <Windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif
#endif

static const int MASK_SUPERSAMPLE = 4;

static void applyWindowRoundedMask(QWindow *window, int radiusPx)
{
    if (!window) return;
#if defined(Q_OS_WIN)
    window->setMask(QRegion());
    return;
#else
    if (radiusPx <= 0) {
        window->setMask(QRegion());
        return;
    }
    const int w = window->width();
    const int h = window->height();
    if (w <= 0 || h <= 0) return;

    const int sw = w * MASK_SUPERSAMPLE;
    const int sh = h * MASK_SUPERSAMPLE;
    const int sr = radiusPx * MASK_SUPERSAMPLE;

    QImage hi(sw, sh, QImage::Format_ARGB32);
    hi.fill(Qt::transparent);
    QPainter painter(&hi);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(QRect(0, 0, sw, sh), sr, sr);
    painter.end();

    QImage scaled = hi.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage mono(w, h, QImage::Format_Mono);
    mono.setColorCount(2);
    mono.setColor(0, 0xFFFFFFFF);
    mono.setColor(1, 0xFF000000);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int alpha = qAlpha(scaled.pixel(x, y));
            mono.setPixel(x, y, alpha > 128 ? 1 : 0);
        }
    }
    QBitmap bitmap = QBitmap::fromImage(mono);
    window->setMask(QRegion(bitmap));
#endif
}

#if defined(Q_OS_WIN)
static void applyWindows11RoundedCorners(QWindow *window)
{
    if (!window) return;
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd) return;
    DWORD preference = DWMWCP_ROUND;
    ::DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
}
#endif

void MainWindowSetup::setup(QQuickWindow *window)
{
    if (!window) return;

    auto updateMask = [window]() {
        const bool maximized = (window->visibility() == QWindow::Maximized);
        applyWindowRoundedMask(window, maximized ? 0 : 10);
    };

    QObject::connect(window, &QWindow::widthChanged, window, updateMask);
    QObject::connect(window, &QWindow::heightChanged, window, updateMask);
#if defined(Q_OS_WIN)
    QObject::connect(window, &QWindow::visibilityChanged, window, [window, updateMask]() {
        QTimer::singleShot(16, window, updateMask);
    });
#else
    QObject::connect(window, &QWindow::visibilityChanged, window, updateMask);
#endif

#if defined(Q_OS_WIN)
    QTimer::singleShot(80, window, [window, updateMask]() {
        window->show();
        window->raise();
        window->requestActivate();
        applyWindows11RoundedCorners(window);
        updateMask();
    });
#else
    window->show();
    window->raise();
    window->requestActivate();
    updateMask();
#endif
}
