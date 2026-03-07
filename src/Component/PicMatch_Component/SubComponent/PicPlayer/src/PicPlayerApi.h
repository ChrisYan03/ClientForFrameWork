#ifndef PICPLAYERAPI_H
#define PICPLAYERAPI_H

#include "PicPlayerDataDef.h"

#ifdef __cplusplus
extern "C" {
#endif

// 初始化
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_Init();

// 反初始化（停止当前业务时调用，仅终止 GLFW，不释放 HandleManager，以便再次进入业务）
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_UnInit();

// 应用退出时调用，释放 HandleManager 等全局资源
PICPLAYER_API void PICPLAYER_CALL PicPlayer_Shutdown();

// 注册消息回调
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_RegisterCallback(int handle, PlayerMsgCallback callback, void* pUser);

// 创建对象实例
PICPLAYER_API int PICPLAYER_CALL PicPlayer_CreateInstance(int cacheNum = 10);

// 销毁指定对象实例
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_DestroyInstance(int handle);

// 注册窗口句柄
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_RegisterWindow(int handle, Window_ShowID winshowId);

// 由宿主在嵌入区域 resize 时调用，通知播放器窗口期望尺寸（如最大化时），渲染循环会应用并更新视口
PICPLAYER_API void PICPLAYER_CALL PicPlayer_SetWindowSize(int handle, int width, int height);

// 开始播放
PICPLAYER_API void PICPLAYER_CALL PicPlayer_Play(int handle);

// 设置主题背景色（OpenGL 清除色与 ImGui 窗口背景），r/g/b 范围 0.0~1.0，由宿主换肤时调用
PICPLAYER_API void PICPLAYER_CALL PicPlayer_SetBackgroundColor(float r, float g, float b);

// 输入图片数据
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_InputPicData(int handle, int iType, void* picData);

// 输入人脸识别结果数据
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_InputFaceRecogResult(int handle, void* recogResult);

#ifdef __cplusplus
}
#endif

#endif // PICPLAYERAPI_H

