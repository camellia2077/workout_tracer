// src/main_app.cpp

#include "controller/ActionHandler.hpp" // 引入 ActionHandler 和 AppConfig
#include <iostream>
#include <string>
#include <optional>
#include <filesystem>
#include <limits>

// 引入 Windows 头文件以支持编码设置
#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
    // 设置 Windows 控制台的输入和输出编码为 UTF-8
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // 1. 打印欢迎界面
    std::cout << "=================================================" << std::endl;
    std::cout << "    Workout Log Processor - Interactive Mode" << std::endl;
    std::cout << "=================================================" << std::endl;
    std::cout << std::endl;

    AppConfig config;
    std::string user_choice;

    // 2. [步骤 1/3] 获取日志文件或文件夹路径
    while (true) {
        std::cout << "[步骤 1/3] 请输入您的日志文件或文件夹路径: ";
        std::getline(std::cin, config.log_filepath);
        if (std::filesystem::exists(config.log_filepath)) {
            break;
        } else {
            std::cerr << "\n[错误] 路径不存在。请检查后重试。\n" << std::endl;
        }
    }
    std::cout << "-> 日志路径已设定: " << config.log_filepath << std::endl << std::endl;

    // 3. [步骤 2/3] 让用户选择是“转换”还是“仅验证”
    while (true) {
        // [MODIFIED] 更新了用户提示
        std::cout << "[步骤 2/3] 请选择操作 (1: 转换日志为JSON, 2: 仅验证日志格式): ";
        std::getline(std::cin, user_choice);
        if (user_choice == "1") {
            // [MODIFIED] 设置 action 为 Convert
            config.action = ActionType::Convert;
            break;
        } else if (user_choice == "2") {
            // [MODIFIED] 设置 action 为 Validate
            config.action = ActionType::Validate;
            break;
        } else {
            std::cerr << "\n[错误] 输入无效，请输入 '1' 或 '2'。\n" << std::endl;
        }
    }
    // [MODIFIED] 更新了确认信息
    std::cout << "-> 已选择: " << (config.action == ActionType::Convert ? "转换日志为JSON" : "仅验证日志格式") << "。\n" << std::endl;
    
    // [MODIFIED] 仅在“转换”模式下才询问年份
    if (config.action == ActionType::Convert) {
        // 4. [步骤 3/3] 获取可选的年份信息
        while (true) {
            std::cout << "[步骤 3/3] 请输入4位数的年份 (例如 2024), 或直接按 Enter 键使用当年: ";
            std::getline(std::cin, user_choice);

            if (user_choice.empty()) {
                config.specified_year = std::nullopt;
                break;
            } else {
                try {
                    int year = std::stoi(user_choice);
                    if (year > 1900 && year < 3000) {
                        config.specified_year = year;
                        break;
                    } else {
                        std::cerr << "\n[错误] 请输入一个合理的年份 (如 1900-3000之间)。\n" << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "\n[错误] 输入无效，请输入纯数字年份。\n" << std::endl;
                }
            }
        }
         std::cout << "-> 年份设定完成。\n" << std::endl;
    }

    // 5. 自动填充剩余的配置路径
    std::cout << "正在自动配置其余路径..." << std::endl;
    std::filesystem::path exe_path = argv[0];
    config.base_path = exe_path.parent_path().string();
    config.mapping_path = "mapping.json";
    std::cout << "-> 程序根路径: " << config.base_path << std::endl;
    std::cout << "-> 映射文件路径: " << config.mapping_path << std::endl << std::endl;

    // 6. 创建并运行 ActionHandler
    std::cout << "配置完成，开始执行...\n" << std::endl;
    ActionHandler handler;
    bool success = handler.run(config);

    // 7. 显示最终结果并等待用户退出
    if (success) {
        std::cout << "\n操作成功完成！" << std::endl;
    } else {
        std::cerr << "\n操作失败。请检查上方的错误信息。" << std::endl;
    }

    std::cout << "\n按 Enter 键退出...";
    std::cin.get();

    return success ? 0 : 1;
}