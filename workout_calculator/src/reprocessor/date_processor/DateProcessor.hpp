#ifndef DATE_PROCESSOR_H
#define DATE_PROCESSOR_H

#include <vector>
#include "common/parsed_data.hpp"

class DateProcessor {
public:
    /**
     * @brief 为数据补全完整的年份，并将日期格式化为 YYYY-MM-DD。
     * @param allData 要处理的数据的引用。
     * @param yearToUse 要使用的4位数年份。
     */
    static void completeDates(std::vector<DailyData>& allData, int yearToUse);
};

#endif // DATE_PROCESSOR_H