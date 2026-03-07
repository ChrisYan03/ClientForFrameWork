#pragma once

#include <string>
#include <stdexcept>
#include <system_error>

namespace ClientForFrame {

// ==============================================================================
// Error Codes
// ==============================================================================
enum class ErrorCode {
    Success = 0,
    InvalidArgument = 1,
    OutOfMemory = 2,
    FileNotFound = 3,
    InvalidOperation = 4,
    InitializationFailed = 5,
    ResourceLoadFailed = 6,
    InvalidState = 7,
    Timeout = 8,
    UnknownError = 999
};

// ==============================================================================
// Exception Classes
// ==============================================================================
class Exception : public std::runtime_error {
public:
    explicit Exception(ErrorCode code, const std::string& message)
        : std::runtime_error(message)
        , m_code(code)
        , m_what(message)
    {}

    ErrorCode code() const noexcept { return m_code; }
    const char* what() const noexcept override { return m_what.c_str(); }

private:
    ErrorCode m_code;
    std::string m_what;
};

// Specific exception types for common errors
class InvalidArgumentException : public Exception {
public:
    explicit InvalidArgumentException(const std::string& message)
        : Exception(ErrorCode::InvalidArgument, message) {}
};

class OutOfMemoryException : public Exception {
public:
    explicit OutOfMemoryException(const std::string& message)
        : Exception(ErrorCode::OutOfMemory, message) {}
};

class FileNotFoundException : public Exception {
public:
    explicit FileNotFoundException(const std::string& message)
        : Exception(ErrorCode::FileNotFound, message) {}
};

class InitializationFailedException : public Exception {
public:
    explicit InitializationFailedException(const std::string& message)
        : Exception(ErrorCode::InitializationFailed, message) {}
};

// ==============================================================================
// Error Handler Class
// ==============================================================================
class ErrorHandler {
public:
    // Log error message
    static void logError(ErrorCode code, const std::string& message);
    static void logError(const std::string& message);

    // Log warning message
    static void logWarning(const std::string& message);

    // Log info message
    static void logInfo(const std::string& message);

    // Throw exception with error code
    [[noreturn]] static void throwError(ErrorCode code, const std::string& message);

    // Convert error code to string
    static std::string errorCodeToString(ErrorCode code);

    // Get last error message (thread-local)
    static std::string getLastError();
    static void setLastError(const std::string& error);

private:
    // Helper to format error messages
    static std::string formatErrorMessage(ErrorCode code, const std::string& message);
};

// ==============================================================================
// Error Handling Macros
// ==============================================================================
#define THROW_ERROR(code, message) \
    ClientForFrame::ErrorHandler::throwError(code, message)

#define THROW_IF(condition, code, message) \
    do { \
        if (condition) { \
            THROW_ERROR(code, message); \
        } \
    } while(0)

#define THROW_IF_NULL(ptr, message) \
    THROW_IF((ptr) == nullptr, ClientForFrame::ErrorCode::InvalidArgument, message)

#define LOG_ERROR(message) \
    ClientForFrame::ErrorHandler::logError(message)

#define LOG_WARNING(message) \
    ClientForFrame::ErrorHandler::logWarning(message)

#define LOG_INFO(message) \
    ClientForFrame::ErrorHandler::logInfo(message)

} // namespace ClientForFrame