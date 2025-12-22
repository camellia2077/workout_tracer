// cli/CommandLineParser.hpp

#ifndef COMMAND_LINE_PARSER_H
#define COMMAND_LINE_PARSER_H

#include "controller/ActionHandler.hpp" 
#include <optional>
#include <string>

class CommandLineParser {
public:
    // [Facade] 对外只暴露这一个简单的静态接口
    static std::optional<AppConfig> parse(int argc, char* argv[]);

private:
    // [Helper] 负责打印帮助
    static void printUsage(const char* programName);
    
    // [Helper] 专门负责打印版本信息
    static void printVersion();

    // [Helper] 核心逻辑分离：负责计算配置文件的物理路径
    // 将 "寻找 config 文件夹" 的逻辑从解析逻辑中剥离出来
    static void resolveConfigPaths(AppConfig& config, const char* executablePath);
};

#endif // COMMAND_LINE_PARSER_H