// src/main_app.cpp

#include "controller/ActionHandler.hpp"
#include <iostream>
#include <string>
#include <optional>
#include <filesystem>
#include <limits>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "=================================================" << std::endl;
    std::cout << "    Workout Log Processor - Interactive Mode" << std::endl;
    std::cout << "=================================================" << std::endl;
    std::cout << std::endl;

    AppConfig config;
    std::string user_choice;

    // [步骤 1/2] 获取输入路径
    std::cout << "[步骤 1/2] 请输入要处理的路径: ";
    std::getline(std::cin, config.log_filepath);
    while (!std::filesystem::exists(config.log_filepath)) {
        std::cerr << "\n[错误] 路径不存在。请检查后重试。\n" << std::endl;
        std::cout << "[步骤 1/2] 请输入要处理的路径: ";
        std::getline(std::cin, config.log_filepath);
    }
    std::cout << "-> 输入路径已设定: " << config.log_filepath << std::endl << std::endl;

    // [步骤 2/2] 让用户选择操作
    while (true) {
        std::cout << "[步骤 2/2] 请选择操作 (1: 转换, 2: 验证, 3: 插入到数据库): ";
        std::getline(std::cin, user_choice);
        if (user_choice == "1") { config.action = ActionType::Convert; break; }
        if (user_choice == "2") { config.action = ActionType::Validate; break; }
        if (user_choice == "3") { config.action = ActionType::Insert; break; }
        std::cerr << "\n[错误] 输入无效，请输入 '1', '2' 或 '3'。\n" << std::endl;
    }
    std::cout << "-> 操作已选择。\n" << std::endl;

    // 自动配置路径
    std::cout << "正在自动配置其余路径..." << std::endl;
    std::filesystem::path exe_path = argv[0];
    config.base_path = exe_path.parent_path().string();
    config.mapping_path = "mapping.json";
    std::cout << "-> 程序根路径: " << config.base_path << std::endl;
    std::cout << "-> 映射文件路径: " << config.mapping_path << std::endl << std::endl;

    std::cout << "配置完成，开始执行...\n" << std::endl;
    
    // [MODIFIED] 直接调用 ActionHandler 的静态 run 方法
    bool success = ActionHandler::run(config);

    if (success) {
        std::cout << "\n操作成功完成！" << std::endl;
    } else {
        std::cerr << "\n操作失败。请检查上方的错误信息。" << std::endl;
    }

    std::cout << "\n按 Enter 键退出...";
    std::cin.get();

    return success ? 0 : 1;
}