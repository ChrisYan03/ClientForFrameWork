#ifndef PICPLAYERWINDOWFORMAC_H
#define PICPLAYERWINDOWFORMAC_H

#ifdef __cplusplus
extern "C" {
#endif

bool GetWindowSizeForMac(void* hwnd, int& width, int& height);
bool SetChildWindow(void* parentWnd, void* childWnd);
void UpdateChildWindowFrameForMac(void* parentWnd, void* childWnd);
void RemoveChildWindowForMac(void* parentWnd, void* childWnd);

#ifdef __cplusplus
}
#endif

#endif // PICPLAYERWINDOWFORMAC_H

