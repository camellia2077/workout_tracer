// main_cli.cpp

#include "cli/CommandLineParser.hpp"
#include "controller/ActionHandler.hpp"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
    // Windows 控制台编码设置
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // 1. 使用新的 CLI 解析器处理参数
    auto configOpt = CommandLineParser::parse(argc, argv);
    if (!configOpt.has_value()) {
        return 1; // 参数错误或显示了帮助信息，退出
    }

    // 2. 执行核心逻辑
    bool success = ActionHandler::run(configOpt.value());

    return success ? 0 : 1;
}