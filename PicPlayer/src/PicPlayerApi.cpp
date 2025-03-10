#include "PicPlayerApi.h"

PICPLAYER_API bool PICPLAYER_CALL PicPlayer_Init()
{
    return true;
}

PICPLAYER_API bool PICPLAYER_CALL PicPlayer_UnInit()
{
    return true;
}

// 注册消息回调
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_RegisterCallback(int handle, PlayerMsgCallback callback, void* pUser)
{
    return true;
}

// 创建对象实例
PICPLAYER_API int PICPLAYER_CALL PicPlayer_CreateInstance(int cacheNum)
{
    return -1;
}

// 销毁指定对象实例
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_DestroyInstance(int handle)
{
    return true;
}

// 注册窗口句柄
PICPLAYER_API int PICPLAYER_CALL PicPlayer_RegisterWindow(int handle, Window_ShowID winshowId)
{
    return 0;
}

// 开始播放
PICPLAYER_API void PICPLAYER_CALL PicPlayer_Play(int handle)
{

}

// 输入图片数据
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_InputPicData(int handle, int iType, void* picData)
{
    return true;
}
