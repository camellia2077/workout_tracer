# cmake/TargetSetup.cmake

# --- 1. 编译器通用设置 ---
function(setup_compiler_flags)
    # 强制开启颜色输出
    set(CMAKE_COLOR_MAKEFILE ON PARENT_SCOPE)
    if(NOT MSVC)
        add_compile_options(-fdiagnostics-color=always)
    endif()

    # Warning Level Handling
    if(WARNING_LEVEL STREQUAL "STRICT")
        message(STATUS "Strict warnings enabled: -Wall -Wextra -Werror")
        add_compile_options(-Wall -Wextra -Werror)
    elseif(WARNING_LEVEL STREQUAL "NONE")
        message(STATUS "Minimal warnings enabled: -w")
        add_compile_options(-w)
    endif()

    # Release 优化
    if(FAST_BUILD)
        message(STATUS "Fast build enabled: Disabling optimizations (-O0)")
        set(CMAKE_CXX_FLAGS_RELEASE "-O0" PARENT_SCOPE)
        set(CMAKE_CXX_FLAGS_DEBUG "-O0" PARENT_SCOPE)
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os" PARENT_SCOPE)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s" PARENT_SCOPE)
    endif()
endfunction()

# --- 2. 定义核心构建函数 ---
function(configure_workout_target TARGET_NAME ENTRY_POINT_FILE SOURCE_LIST)
    # 创建可执行文件
    add_executable(
        ${TARGET_NAME}
        ${ENTRY_POINT_FILE}
        ${SOURCE_LIST}
    )

    # 设置头文件包含路径
    target_include_directories(
        ${TARGET_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # 预编译头文件
    target_precompile_headers(
        ${TARGET_NAME}
        PRIVATE
        src/pch.hpp
    )

    # 链接第三方库
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
        cjson
        SQLite::SQLite3
    )

    # --- 复制配置文件逻辑 (已修改) ---
    # [MODIFIED] 源路径改为根目录下的 config/mapping.json，不再从 src/config 查找
    set(CONFIG_FILE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/config/mapping.json)
    
    add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/config"
        COMMAND ${CMAKE_COMMAND} -E copy
                "${CONFIG_FILE_SOURCE}"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>/config/mapping.json"
        COMMENT "Copying config for ${TARGET_NAME} from root directory"
    )
endfunction()

# --- 3. 添加 Clang-Tidy 和 Clang-Format 目标 ---
function(add_lint_targets SOURCE_LIST)
    # 查找 clang-format
    find_program(CLANG_FORMAT "clang-format")
    if(CLANG_FORMAT)
        add_custom_target(
            format
            COMMAND ${CLANG_FORMAT} -i ${SOURCE_LIST}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Formatting all source files..."
        )
    endif()

    # 查找 clang-tidy
    find_program(CLANG_TIDY "clang-tidy")
    if(CLANG_TIDY)
        # 注意: clang-tidy 通常在编译数据库 (compile_commands.json) 所在位置运行效果最好
        add_custom_target(
            tidy
            COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} ${SOURCE_LIST}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running clang-tidy on all source files..."
        )
    endif()
endfunction()
