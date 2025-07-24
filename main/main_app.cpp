#include "controller/ActionHandler.h" // 引入 ActionHandler 和 AppConfig
#include <iostream>
#include <string>
#include <optional>
#include <filesystem>
#include <limits>

// 引入 Windows 头文件以支持编码设置
#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief 清理标准输入缓冲区。
 */
void clearCinBuffer() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

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

    // 2. [步骤 1/4] 获取日志文件路径
    while (true) {
        std::cout << "[步骤 1/4] 请输入您的日志文件路径: ";
        std::getline(std::cin, config.log_filepath);

        if (std::filesystem::exists(config.log_filepath) && !std::filesystem::is_directory(config.log_filepath)) {
            break;
        } else {
            std::cerr << "\n[错误] 文件未找到或路径无效。请检查后重试。\n" << std::endl;
        }
    }
    std::cout << "-> 日志文件已设定: " << config.log_filepath << std::endl << std::endl;

    // 3. [步骤 2/4] 让用户选择是“处理”还是“仅验证”
    while (true) {
        std::cout << "[步骤 2/4] 请选择操作 (1: 处理文件, 2: 仅验证格式): ";
        std::getline(std::cin, user_choice);

        if (user_choice == "1") {
            config.validate_only = false;
            std::cout << "-> 已选择: 处理文件。\n" << std::endl;
            break;
        } else if (user_choice == "2") {
            config.validate_only = true;
            std::cout << "-> 已选择: 仅验证格式。\n" << std::endl;
            break;
        } else {
            std::cerr << "\n[错误] 输入无效，请输入 '1' 或 '2'。\n" << std::endl;
        }
    }
    
    // **已修改：仅在完整处理模式下，才询问输出模式和年份**
    if (!config.validate_only) {
        // 4. [步骤 3/4] (新) 让用户选择输出模式
        while (true) {
            std::cout << "[步骤 3/4] 请选择输出模式:" << std::endl;
            std::cout << "  1: 输出格式化文件并存入数据库 (默认)" << std::endl;
            std::cout << "  2: 仅输出格式化文件" << std::endl;
            std::cout << "  3: 仅存入数据库" << std::endl;
            std::cout << "请输入选项 (1-3): ";
            std::getline(std::cin, user_choice);

            if (user_choice == "2") {
                config.output_mode = OutputMode::FILE_ONLY;
                std::cout << "-> 已选择: 仅输出文件。\n" << std::endl;
                break;
            } else if (user_choice == "3") {
                config.output_mode = OutputMode::DB_ONLY;
                std::cout << "-> 已选择: 仅存入数据库。\n" << std::endl;
                break;
            } else if (user_choice == "1" || user_choice.empty()) {
                config.output_mode = OutputMode::ALL;
                std::cout << "-> 已选择: 输出文件并存入数据库。\n" << std::endl;
                break;
            } else {
                std::cerr << "\n[错误] 输入无效，请输入 '1', '2' 或 '3'。\n" << std::endl;
            }
        }

        // 5. [步骤 4/4] 获取可选的年份信息
        while (true) {
            std::cout << "[步骤 4/4] 请输入4位数的年份 (例如 2024), 或直接按 Enter 键使用当年: ";
            std::getline(std::cin, user_choice);

            if (user_choice.empty()) {
                config.specified_year = std::nullopt;
                std::cout << "-> 好的，将使用当前系统年份。" << std::endl << std::endl;
                break;
            } else {
                try {
                    int year = std::stoi(user_choice);
                    if (year > 1900 && year < 3000) {
                        config.specified_year = year;
                        std::cout << "-> 年份已设定为: " << year << std::endl << std::endl;
                        break;
                    } else {
                        std::cerr << "\n[错误] 请输入一个合理的年份 (如 1900-3000之间)。\n" << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "\n[错误] 输入无效，请输入纯数字年份。\n" << std::endl;
                }
            }
        }
    }

    // 6. 自动填充剩余的配置路径
    std::cout << "正在自动配置其余路径..." << std::endl;
    std::filesystem::path exe_path = argv[0];
    config.db_path = (exe_path.parent_path() / "workouts.sqlite3").string();
    config.mapping_path = "mapping.json";
    std::cout << "-> 数据库路径: " << config.db_path << std::endl;
    std::cout << "-> 映射文件路径: " << config.mapping_path << std::endl;
    std::cout << std::endl;

    // 7. 创建并运行 ActionHandler
    std::cout << "配置完成，开始执行...\n" << std::endl;
    ActionHandler handler;
    bool success = handler.run(config);

    // 8. 显示最终结果并等待用户退出
    if (success) {
        std::cout << "\n操作成功完成！" << std::endl;
    } else {
        std::cerr << "\n操作失败。请检查上方的错误信息。" << std::endl;
    }

    std::cout << "\n按 Enter 键退出...";
    // 确保在 getline 之后正确地等待输入
    std::cin.get();

    return success ? 0 : 1;
}