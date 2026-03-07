#include "ErrorHandler.h"
#include "LogUtil.h"
#include <string>
#include <mutex>

namespace ClientForFrame {

// Thread-local storage for last error
thread_local std::string t_lastError;

// ==============================================================================
// Error Handler Implementation
// ==============================================================================
void ErrorHandler::logError(ErrorCode code, const std::string& message) {
    std::string fullMessage = formatErrorMessage(code, message);
    ::LOG_ERROR("{}", fullMessage);
    setLastError(fullMessage);
}

void ErrorHandler::logError(const std::string& message) {
    ::LOG_ERROR("{}", message);
    setLastError(message);
}

void ErrorHandler::logWarning(const std::string& message) {
    ::LOG_WARNING("{}", message);
}

void ErrorHandler::logInfo(const std::string& message) {
    ::LOG_INFO("{}", message);
}

[[noreturn]] void ErrorHandler::throwError(ErrorCode code, const std::string& message) {
    std::string fullMessage = formatErrorMessage(code, message);
    ::LOG_ERROR("{}", fullMessage);
    setLastError(fullMessage);
    throw Exception(code, fullMessage);
}

std::string ErrorHandler::errorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::Success:
            return "Success";
        case ErrorCode::InvalidArgument:
            return "InvalidArgument";
        case ErrorCode::OutOfMemory:
            return "OutOfMemory";
        case ErrorCode::FileNotFound:
            return "FileNotFound";
        case ErrorCode::InvalidOperation:
            return "InvalidOperation";
        case ErrorCode::InitializationFailed:
            return "InitializationFailed";
        case ErrorCode::ResourceLoadFailed:
            return "ResourceLoadFailed";
        case ErrorCode::InvalidState:
            return "InvalidState";
        case ErrorCode::Timeout:
            return "Timeout";
        case ErrorCode::UnknownError:
        default:
            return "UnknownError";
    }
}

std::string ErrorHandler::getLastError() {
    return t_lastError;
}

void ErrorHandler::setLastError(const std::string& error) {
    t_lastError = error;
}

std::string ErrorHandler::formatErrorMessage(ErrorCode code, const std::string& message) {
    return "[" + errorCodeToString(code) + "] " + message;
}

} // namespace ClientForFrame