#ifndef PICPLAYERDATADEF_H
#define PICPLAYERDATADEF_H

#include "PicPlayerGlobal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(PICPLAYER_CALL *PlayerMsgCallback)(int handle, int iMsg, int iRes, void* pData, void* pUser);

using Window_ShowID = decltype(sizeof(void*));


#ifdef __cplusplus
}
#endif
#endif // PICPLAYERDATADEF_H
