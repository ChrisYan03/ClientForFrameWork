// File: e:\ClientForFrameWork\src\Utils\LogUtil.h
#ifndef LOGUTIL_H
#define LOGUTIL_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace LogUtil {
    inline std::shared_ptr<spdlog::logger> getLogger(const std::string& name = "app") {
        static std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> s_loggers;
        static std::mutex s_mutex;
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_loggers.find(name);
        if (it != s_loggers.end())
            return it->second;
        try {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            std::string logFilePath = "logs/" + name + ".log";
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, true);
            auto logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{console_sink, file_sink});
            logger->set_level(spdlog::level::trace);
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");
            logger->flush_on(spdlog::level::trace);
            if (s_loggers.empty())
                spdlog::set_default_logger(logger);
            s_loggers[name] = logger;
            return logger;
        } catch (const spdlog::spdlog_ex& ex) {
            if (!s_loggers.empty())
                spdlog::error("Log initialization failed for {}: {}", name, ex.what());
            return s_loggers.empty() ? nullptr : s_loggers.begin()->second;
        }
    }

    inline void initLogger(const std::string& name = "app") {
        getLogger(name);
    }
    
    inline void setLogLevel(spdlog::level::level_enum level) {
        spdlog::set_level(level);
    }
    
    inline void flushLogs() {
        spdlog::flush_every(std::chrono::seconds(1));
    }
}

// 辅助宏：从完整路径中提取文件名
#define GET_FILENAME(path) (strrchr(path, '\\') ? strrchr(path, '\\') + 1 : strrchr(path, '/') ? strrchr(path, '/') + 1 : path)

// 定义日志宏以包含文件名、行号和函数名
#define LOG_TRACE(format, ...)    spdlog::trace("[{}:{}:{}] " #format, GET_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...)    spdlog::debug("[{}:{}:{}] " #format, GET_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_INFO(format, ...)     spdlog::info("[{}:{}:{}] " #format, GET_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_WARN(format, ...)     spdlog::warn("[{}:{}:{}] " #format, GET_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)    spdlog::error("[{}:{}:{}] " #format, GET_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_CRITICAL(format, ...) spdlog::critical("[{}:{}:{}] " #format, GET_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)

#endif // LOGUTIL_H