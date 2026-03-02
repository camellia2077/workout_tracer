#include "VolumeCalculator.h"
#include <numeric> // 用于 std::accumulate

void VolumeCalculator::calculateVolume(std::vector<DailyData>& allData) {
    // 遍历每一天的数据
    for (auto& dailyData : allData) {
        // 遍历当天的每一个项目
        for (auto& project : dailyData.projects) {
            // 计算总次数
            // std::accumulate可以高效地对容器内的元素求和
            double totalReps = std::accumulate(project.reps.begin(), project.reps.end(), 0);

            // 计算容量并存入结构体
            project.volume = project.weight * totalReps;
        }
    }
}