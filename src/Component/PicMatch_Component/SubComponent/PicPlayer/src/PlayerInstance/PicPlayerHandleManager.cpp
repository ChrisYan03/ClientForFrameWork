#include "PicPlayerHandleManager.h"
#include <iostream>
#include <memory>

//private:
PicPlayerHandleManager::PicPlayerHandleManager()
{
    m_handleRegister.InitHandleRegister();
}

PicPlayerHandleManager::~PicPlayerHandleManager()
{
    m_handleRegister.ReleaseAll();
}

// public
int PicPlayerHandleManager::CreatePlayer(int cacheNum)
{
    int playerHandle = -1;
    try {
        std::unique_ptr<PicPlayer> playerPtr = std::make_unique<PicPlayer>(cacheNum);
        playerHandle = m_handleRegister.RegisterObjInstance(std::move(playerPtr));
        if (-1 == playerHandle) {
            return playerHandle;
        }
        // playerPtr 现在是 nullptr，所有权已转移
        m_handleRegister.GetObjInstance(playerHandle)->SetHandle(playerHandle);
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return playerHandle;
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return playerHandle;
    }
    return playerHandle;
}

void PicPlayerHandleManager::SetPlayerCallback(int handle, PlayerMsgCallback callback, void* pUser)
{
    PicPlayer* pPlayer = instance()->GetPlayer(handle);
    if (pPlayer) {
        pPlayer->SetPicCallback(callback, pUser);
    }
}

PicPlayer* PicPlayerHandleManager::GetPlayer(int handle)
{
    return m_handleRegister.GetObjInstance(handle);
}

void PicPlayerHandleManager::RemovePlayer(int handle)
{
    m_handleRegister.EarseObjInstance(handle);
}

