// reprocessor/preprocessor/volume_calculator/VolumeCalculator.hpp
#ifndef DATA_PROCESSOR_H
#define DATA_PROCESSOR_H

#include <vector>
#include "common/parsed_data.hpp"

class VolumeCalculator {
public:
    // 计算每个项目的容量
    // 传入一个可修改的数据引用，直接在原始数据上进行处理
    static void calculateVolume(std::vector<DailyData>& allData);

private:
    // [NEW] 辅助函数：计算单个项目的总容量，并更新其中每组的 volume 字段
    static double calculateProjectVolume(std::vector<SetData>& sets);
};

#endif // DATA_PROCESSOR_H