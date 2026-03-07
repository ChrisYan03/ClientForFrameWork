#pragma once

namespace CrashpadInit {

/**
 * 在 main() 最早处调用，用于启动 Crashpad 崩溃捕获。
 * 需在 QApplication 等可能崩溃的代码之前调用。
 * 未定义 USE_CRASHPAD 时为空实现。
 */
void initialize(int argc, char *argv[]);

} // namespace CrashpadInit
