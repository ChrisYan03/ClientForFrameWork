#include "PicPlayerApi.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "PicPlayerHandleManager.h"
#include "PicPlayer.h"
#include "PlayerInstance/PicPlayerShowWindow.h"
#include "PicPlayerLog.h"

namespace {
static bool s_glfwInited = false;
}

PICPLAYER_API bool PICPLAYER_CALL PicPlayer_Init()
{
    if (s_glfwInited)
        return true;
    LogUtil::initLogger("PicPlayer");
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }
    s_glfwInited = true;
    return true;
}

PICPLAYER_API bool PICPLAYER_CALL PicPlayer_UnInit()
{
    if (!s_glfwInited)
        return true;
    glfwTerminate();
    s_glfwInited = false;
    return true;
}

PICPLAYER_API void PICPLAYER_CALL PicPlayer_Shutdown()
{
    if (s_glfwInited) {
        glfwTerminate();
        s_glfwInited = false;
    }
    PicPlayerHandleManager::free();
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

PICPLAYER_API void PICPLAYER_CALL PicPlayer_SetWindowSize(int handle, int width, int height)
{
    PicPlayer* player = PicPlayerHandleManager::instance()->GetPlayer(handle);
    if (player) {
        PicPlayerGui* gui = player->GetGui();
        if (gui) {
            PicPlayerShowWindow* showWin = static_cast<PicPlayerShowWindow*>(gui);
            showWin->SetDesiredSize(width, height);
        }
    }
}

// 开始播放
PICPLAYER_API void PICPLAYER_CALL PicPlayer_Play(int handle)
{
    PicPlayer* player = PicPlayerHandleManager::instance()->GetPlayer(handle);
    if (player) {
        player->StartPlayer();
    }
}

PICPLAYER_API void PICPLAYER_CALL PicPlayer_SetBackgroundColor(float r, float g, float b)
{
    PicPlayerShowWindow::SetThemeBackgroundColor(r, g, b);
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

PICPLAYER_API bool PICPLAYER_CALL PicPlayer_InputFaceRecogResult(int handle, void* recogResult)
{
    PicPlayer* player = PicPlayerHandleManager::instance()->GetPlayer(handle);
    if (player) {
        player->InputFaceRecogResult(recogResult);
        return true;
    }
    return false;
}