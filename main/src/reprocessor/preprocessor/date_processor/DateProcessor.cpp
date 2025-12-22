// reprocessor/preprocessor/date_processor/DateProcessor.cpp
#include "DateProcessor.hpp"

void DateProcessor::completeDates(std::vector<DailyData>& allData, int yearToUse) {
    for (auto& daily : allData) {
        if (daily.date.length() == 4) { // 只处理 "MMDD" 格式
            std::string month = daily.date.substr(0, 2);
            std::string day = daily.date.substr(2, 2);
            daily.date = std::to_string(yearToUse) + "-" + month + "-" + day;
        }
    }
}