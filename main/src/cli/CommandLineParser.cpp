// cli/CommandLineParser.cpp

#include "CommandLineParser.hpp"
#include "common/version.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// 1. Facade 入口：逻辑流非常清晰
std::optional<AppConfig> CommandLineParser::parse(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return std::nullopt;
    }

    std::vector<std::string> args(argv + 1, argv + argc);
    std::string command = args[0];

    // 优先处理“非业务”指令
    if (command == "-h" || command == "--help") {
        printUsage(argv[0]);
        return std::nullopt;
    }
    if (command == "-v" || command == "--version") {
        printVersion();
        return std::nullopt;
    }

    // 开始构建配置对象
    AppConfig config;

    // --- 逻辑块 A: 命令解析 ---
    if (command == "export") {
        if (args.size() > 1) { 
            std::cerr << "Error: 'export' command does not take any additional arguments." << std::endl;
            return std::nullopt;
        }
        config.action = ActionType::Export;
    } 
    else if (command == "validate" || command == "convert" || command == "insert") {
        if (args.size() < 2) {
            std::cerr << "Error: '" << command << "' command requires a <path> argument." << std::endl;
            return std::nullopt;
        }
        
        config.log_filepath = args[1]; // 设置文件路径
        
        if (command == "validate") config.action = ActionType::Validate;
        if (command == "convert")  config.action = ActionType::Convert;
        if (command == "insert")   config.action = ActionType::Insert;

        // 严格检查多余参数
        if (args.size() > 2) {
            std::cerr << "Error: Unknown or invalid argument '" << args[2] << "'" << std::endl;
            return std::nullopt;
        }
    } else {
        std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
        printUsage(argv[0]);
        return std::nullopt;
    }

    // --- 逻辑块 B: 环境路径解析 (解耦的关键) ---
    // 无论什么命令，只要不是 help/version，都需要加载环境配置
    // 我们将这部分脏活累活交给 helper 处理
    resolveConfigPaths(config, argv[0]);

    return config;
}

// 2. 独立的路径解析逻辑
void CommandLineParser::resolveConfigPaths(AppConfig& config, const char* executablePath) {
    try {
        // 获取 exe 的绝对路径和所在目录
        fs::path exe_path = fs::absolute(fs::path(executablePath));
        fs::path exe_dir = exe_path.parent_path();

        // 构建 config 目录和 mapping 文件的路径
        fs::path config_dir = exe_dir / "config";
        fs::path mapping_file = config_dir / "mapping.json";

        // 赋值给 config 对象
        config.base_path = exe_dir.string();
        config.mapping_path = mapping_file.string();

        // 检查配置文件是否存在
        if (!fs::exists(config.mapping_path)) {
            std::cerr << "Warning: Configuration file not found at: " << config.mapping_path << std::endl;
            std::cerr << "Please ensure you have created a 'config' folder next to the executable" << std::endl;
            std::cerr << "and placed 'mapping.json' inside it." << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error resolving paths: " << e.what() << std::endl;
        // 这里可以选择抛出异常或设置一个错误状态，视具体需求而定
    }
}

// 3. 版本打印
void CommandLineParser::printVersion() {
    std::cout << BuildInfo::PROJECT_NAME << " version " << BuildInfo::VERSION 
              << " (Built: " << BuildInfo::BUILD_DATE << ")" << std::endl;
}

// 4. 帮助打印 (保持不变)
void CommandLineParser::printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " <command> [<path>] [options]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Description:" << std::endl;
    std::cerr << "  Processes, validates, inserts, or exports workout logs." << std::endl;
    std::cerr << std::endl;
    std::cerr << "Commands:" << std::endl;
    std::cerr << "  validate <path>        Only validate the log file format." << std::endl;
    std::cerr << "  convert <path>         Convert the log file to JSON format." << std::endl;
    std::cerr << "  insert <path>          Insert JSON files into the database." << std::endl;
    std::cerr << "  export                 Export all data from the database to Markdown files." << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -h, --help             Show this help message and exit." << std::endl;
    std::cerr << "  -v, --version          Show version information and exit." << std::endl;
}