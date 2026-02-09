#include "PicPlayerApi.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "PicPlayerHandleManager.h"
#include "PicPlayer.h"

PICPLAYER_API bool PICPLAYER_CALL PicPlayer_Init()
{
    if(!glfwInit()){
        return false;
    }
    return true;
}

PICPLAYER_API bool PICPLAYER_CALL PicPlayer_UnInit()
{
    glfwTerminate();
    PicPlayerHandleManager::free();
    return true;
}

// 注册消息回调
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_RegisterCallback(int handle, PlayerMsgCallback callback, void* pUser)
{
    PicPlayerHandleManager::instance()->SetPlayerCallback(handle, callback, pUser);
    return true;
}

// 创建对象实例
PICPLAYER_API int PICPLAYER_CALL PicPlayer_CreateInstance(int cacheNum)
{
    return PicPlayerHandleManager::instance()->CreatePlayer(cacheNum);
}

// 销毁指定对象实例
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_DestroyInstance(int handle)
{
    PicPlayerHandleManager::instance()->RemovePlayer(handle);
    return true;
}

// 注册窗口句柄
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_RegisterWindow(int handle, Window_ShowID winshowId)
{
    PicPlayer* player = PicPlayerHandleManager::instance()->GetPlayer(handle);
    if (player) {
        player->SetWid(winshowId);
        return true;
    }
    return false;
}

// 开始播放
PICPLAYER_API void PICPLAYER_CALL PicPlayer_Play(int handle)
{
    PicPlayer* player = PicPlayerHandleManager::instance()->GetPlayer(handle);
    if (player) {
        player->StartPlayer();
    }
}

// 输入图片数据
PICPLAYER_API bool PICPLAYER_CALL PicPlayer_InputPicData(int handle, int iType, void* picData)
{
    PicPlayer* player = PicPlayerHandleManager::instance()->GetPlayer(handle);
    if (player) {
        player->InputPicData(iType, picData);
        return true;
    }
    return false;
}

