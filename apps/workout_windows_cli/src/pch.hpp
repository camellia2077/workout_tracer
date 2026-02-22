#pragma once
#ifndef PCH_H
#define PCH_H

// ===================================================================
//  1. C++ 标准库 (Standard Library)
//  最稳定、最庞大、使用最频繁的部分。
// ===================================================================
#include <algorithm>   // 使用次数: 3
#include <cctype>      // 使用次数: 1
#include <cmath>       // 使用次数: 3
#include <filesystem>  // 使用次数: 6
#include <fstream>     // 使用次数: 4
#include <iostream>    // 使用次数: 26
#include <map>         // 使用次数: 6
#include <memory>      // 使用次数: 2
#include <numeric>     // 使用次数: 2
#include <optional>    // 使用次数: 8
#include <regex>       // 使用次数: 3
#include <sstream>     // 使用次数: 4
#include <stdexcept>   // 使用次数: 2
#include <string>      // 使用次数: 19
#include <string_view> // 使用次数: 1
#include <vector>      // 使用次数: 19

// ===================================================================
//  2. 平台与第三方库 (Platform & Third-Party)
//  改动频率低，是 PCH 的理想候选。
// ===================================================================
#include <sqlite3.h> // 使用次数: 5
#ifdef _WIN32
#include <windows.h> // 使用次数: 1
#endif

// ===================================================================
//  3. 项目内部稳定且常用的核心头文件
//  建议仅包含极少修改的核心接口。
// ===================================================================
#include "cli/command_line_parser.hpp" // 使用次数: 1

#include "application/action_handler.hpp"                // 使用次数: 5
#include "application/database_handler.hpp"              // 使用次数: 2
#include "application/file_processor_handler.hpp"        // 使用次数: 2
#include "application/interfaces/i_log_parser.hpp"       // 使用次数: 3
#include "application/interfaces/i_mapping_provider.hpp" // 使用次数: 4

#include "cjson/cJSON.h" // 使用次数: 4

#include "cli/commands/convert_command.hpp"  // 使用次数: 1
#include "cli/commands/export_command.hpp"   // 使用次数: 1
#include "cli/commands/insert_command.hpp"   // 使用次数: 1
#include "cli/commands/validate_command.hpp" // 使用次数: 1
#include "cli/framework/application.hpp"     // 使用次数: 2
#include "cli/framework/command.hpp"         // 使用次数: 5

#include "common/c_json_helper.hpp" // 使用次数: 3
#include "common/file_reader.hpp"   // 使用次数: 3
#include "common/json_reader.hpp"   // 使用次数: 3
#include "common/version.hpp"       // 使用次数: 2

#include "infrastructure/converter/converter.hpp"           // 使用次数: 2
#include "infrastructure/converter/log_parser.hpp"          // 使用次数: 3
#include "infrastructure/converter/project_name_mapper.hpp" // 使用次数: 2

#include "infrastructure/persistence/facade/db_facade.hpp"       // 使用次数: 2
#include "infrastructure/persistence/inserter/data_inserter.hpp" // 使用次数: 2
#include "infrastructure/persistence/manager/db_manager.hpp"     // 使用次数: 2

#include "domain/models/workout_item.hpp"     // 使用次数: 8
#include "domain/services/date_service.hpp"   // 使用次数: 2
#include "domain/services/volume_service.hpp" // 使用次数: 2

#include "infrastructure/config/file_mapping_provider.hpp" // 使用次数: 1

#include "infrastructure/reporting/database/database_manager.hpp" // 使用次数: 2
#include "infrastructure/reporting/facade/report_facade.hpp"      // 使用次数: 2
#include "infrastructure/reporting/formatter/markdown_formatter.hpp" // 使用次数: 2

#include "infrastructure/serializer/serializer.hpp" // 使用次数: 3

#include "infrastructure/validation/validator.hpp" // 使用次数: 4

#endif // PCH_H