// File: e:\ClientForFrameWork\ClientForFrame\src\Utils\LogUtil.h
#ifndef LOGUTIL_H
#define LOGUTIL_H

#include <spdlog.h>
#include <sinks/basic_file_sink.h>
#include <sinks/stdout_color_sinks.h>
#include <memory>
#include <string>

namespace LogUtil {
    inline std::shared_ptr<spdlog::logger> getLogger(const std::string& name = "app") {
        static std::shared_ptr<spdlog::logger> logger = nullptr;
        
        if (!logger) {
            try {
                // 创建控制台和文件双输出日志
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/app.log", true);
                
                logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{console_sink, file_sink});
                
                // 设置全局日志级别
                logger->set_level(spdlog::level::trace);
                
                // 设置日志格式
                logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");
                
                // 立即刷新
                logger->flush_on(spdlog::level::trace);
                
                // 设置全局日志器
                spdlog::set_default_logger(logger);
            } catch (const spdlog::spdlog_ex& ex) {
                spdlog::error("Log initialization failed: {}", ex.what());
            }
        }
        
        return logger;
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
#endif // LOGUTIL_H