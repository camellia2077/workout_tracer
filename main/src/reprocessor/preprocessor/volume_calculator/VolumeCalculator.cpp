// reprocessor/preprocessor/volume_calculator/VolumeCalculator.cpp

#include "VolumeCalculator.hpp"
#include <numeric>

// [NEW] 提取出来的辅助函数，负责处理单个项目的计算逻辑
double VolumeCalculator::calculateProjectVolume(std::vector<SetData>& sets) {
    double totalVolume = 0.0;
    
    for (auto& set : sets) {
        // 如果重量为负（弹力带辅助），不计算容量，容量记为 0
        if (set.weight < 0) {
            set.volume = 0.0;
            continue; 
        }

        set.volume = set.weight * set.reps;
        totalVolume += set.volume;
    }

    return totalVolume;
}

// 主函数现在更加简洁，嵌套层级减少，意图更清晰
void VolumeCalculator::calculateVolume(std::vector<DailyData>& allData) {
    for (auto& dailyData : allData) {
        for (auto& project : dailyData.projects) {
            // 直接调用辅助函数，将计算细节隐藏
            project.totalVolume = calculateProjectVolume(project.sets);
        }
    }
}