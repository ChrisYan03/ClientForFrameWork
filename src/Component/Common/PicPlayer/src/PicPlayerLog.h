#ifndef PICPLAYERLOG_H
#define PICPLAYERLOG_H

// 在 PicPlayer 内使用此头替代 LogUtil.h，使 LOG_* 写入 logs/PicPlayer.log
#include "LogUtil.h"

#undef LOG_TRACE
#undef LOG_DEBUG
#undef LOG_INFO
#undef LOG_WARN
#undef LOG_ERROR
#undef LOG_CRITICAL

#define LOG_TRACE(format, ...)    LogUtil::getLogger("PicPlayer")->trace("[{}:{}:{}] " format, GET_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...)    LogUtil::getLogger("PicPlayer")->debug("[{}:{}:{}] " format, GET_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_INFO(format, ...)     LogUtil::getLogger("PicPlayer")->info("[{}:{}:{}] " format, GET_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_WARN(format, ...)     LogUtil::getLogger("PicPlayer")->warn("[{}:{}:{}] " format, GET_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)    LogUtil::getLogger("PicPlayer")->error("[{}:{}:{}] " format, GET_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_CRITICAL(format, ...) LogUtil::getLogger("PicPlayer")->critical("[{}:{}:{}] " format, GET_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)

#endif
