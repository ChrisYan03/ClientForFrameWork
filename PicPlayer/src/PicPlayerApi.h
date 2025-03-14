#ifndef PICPLAYERAPI_H
#define PICPLAYERAPI_H

#include "PicPlayerDataDef.h"

#ifdef __cplusplus
extern "C" {
#endif

// 初始化
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_Init();

// 反初始化
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_UnInit();

// 注册消息回调
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_RegisterCallback(int handle, PlayerMsgCallback callback, void* pUser);

// 创建对象实例
PICPLAYER_API int PICPLAYER_CALL PicPlayer_CreateInstance(int cacheNum = 10);

// 销毁指定对象实例
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_DestroyInstance(int handle);

// 注册窗口句柄
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_RegisterWindow(int handle, Window_ShowID winshowId);

// 开始播放
PICPLAYER_API void PICPLAYER_CALL PicPlayer_Play(int handle);

// 输入图片数据
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_InputPicData(int handle, int iType, void* picData);


#ifdef __cplusplus
}
#endif

#endif // PICPLAYERAPI_H
