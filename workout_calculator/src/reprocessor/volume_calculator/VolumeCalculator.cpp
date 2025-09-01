// src/reprocessor/volume_calculator/VolumeCalculator.cpp

#include "VolumeCalculator.hpp"
#include <numeric>

void VolumeCalculator::calculateVolume(std::vector<DailyData>& allData) {
    for (auto& dailyData : allData) {
        for (auto& project : dailyData.projects) {
            double totalProjectVolume = 0.0;
            // 遍历项目中的每一组
            for (auto& set : project.sets) {
                // 1. 计算单组的容量
                set.volume = set.weight * set.reps;
                // 2. 累加到项目的总容量中
                totalProjectVolume += set.volume;
            }
            // 3. 将计算出的总容量存入结构体
            project.totalVolume = totalProjectVolume;
        }
    }
}