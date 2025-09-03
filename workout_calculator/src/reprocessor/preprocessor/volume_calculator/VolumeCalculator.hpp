#ifndef DATA_PROCESSOR_H
#define DATA_PROCESSOR_H

#include <vector>
#include "common/parsed_data.hpp"

class VolumeCalculator {
public:
    // 计算每个项目的容量
    // 传入一个可修改的数据引用，直接在原始数据上进行处理
    static void calculateVolume(std::vector<DailyData>& allData);

};

#endif // DATA_PROCESSOR_H