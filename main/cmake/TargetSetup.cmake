# cmake/TargetSetup.cmake

# --- 1. 编译器通用设置 ---
function(setup_compiler_flags)
    # 强制开启颜色输出
    set(CMAKE_COLOR_MAKEFILE ON PARENT_SCOPE)
    if(NOT MSVC)
        add_compile_options(-fdiagnostics-color=always)
    endif()

    # Release 优化
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os" PARENT_SCOPE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s" PARENT_SCOPE)
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
        nlohmann_json::nlohmann_json
        SQLite::SQLite3
    )

    # 复制配置文件逻辑
    set(CONFIG_FILE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/config/mapping.json)
    add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/config"
        COMMAND ${CMAKE_COMMAND} -E copy
                "${CONFIG_FILE_SOURCE}"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>/config/mapping.json"
        COMMENT "Copying config for ${TARGET_NAME}"
    )
endfunction()