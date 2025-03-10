#ifndef PICPLAYERHANDLEMANAGER_H
#define PICPLAYERHANDLEMANAGER_H

#include "PicPlayer.h"
#include "../Utils/HandleRegister.h"
#include "../Utils/Singleton.h"

class PicPlayerHandleManager : public Singleton<PicPlayerHandleManager>
{
    friend class Singleton<PicPlayerHandleManager>;
public:
    int CreatePlayer(int cacheNum);
    void SetPlayerCallback(int handle, PlayerMsgCallback callback, void* pUser);
    PicPlayer* GetPlayer(int handle);
    void RemovePlayer(int handle);

private:
    PicPlayerHandleManager();
    ~PicPlayerHandleManager();

private:
    HandleRegister<PicPlayer> m_handleRegister;
};

#endif // PICPLAYERHANDLEMANAGER_H
