// src/controller/ActionHandler.cpp

#include "ActionHandler.hpp"
#include "FileProcessorHandler.hpp"
#include "DatabaseHandler.hpp"

// <<< [MODIFIED] 在方法定义前添加 "bool ActionHandler::" 以匹配静态声明
bool ActionHandler::run(const AppConfig& config) {
    if (config.action == ActionType::Validate || config.action == ActionType::Convert) {
        FileProcessorHandler fileProcessor;
        return fileProcessor.handle(config);
    } 
    else if (config.action == ActionType::Insert || config.action == ActionType::Export) {
        DatabaseHandler dbHandler;
        return dbHandler.handle(config);
    }
    
    return false; // Or handle unknown action type
}