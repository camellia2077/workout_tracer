// application/database_handler.cpp

#include "application/database_handler.hpp"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

#include "common/file_reader.hpp"
#include "common/json_reader.hpp"
#include "infrastructure/persistence/facade/db_facade.hpp"
#include "infrastructure/persistence/facade/query_facade.hpp"
#include "infrastructure/persistence/manager/db_manager.hpp"
#include "infrastructure/reporting/facade/report_facade.hpp"
#include "infrastructure/serializer/serializer.hpp"

namespace fs = std::filesystem;

auto DatabaseHandler::Handle(const AppConfig& config) -> bool {
  if (config.action_ == ActionType::Insert) {
    std::cout << "Performing database insertion..." << std::endl;

    fs::path db_path = fs::path(config.base_path_) / "workout_logs.sqlite3";
    DbManager db_manager(db_path.string());

    if (!db_manager.Open()) {
      return false;
    }

    std::vector<std::string> json_files =
        FileReader::FindFilesByExtension(config.log_filepath_, ".json");
    if (json_files.empty()) {
      std::cout << "Warning: No .json files found to insert." << std::endl;
      return true;
    }

    int success_count = 0;
    for (const auto& json_path : json_files) {
      std::cout << "--- Inserting file: " << json_path << " ---" << std::endl;
      auto json_data_opt = JsonReader::ReadFile(json_path);
      if (json_data_opt.has_value()) {
        auto training_data =
            Serializer::Deserialize(json_data_opt.value().get());

        if (DbFacade::InsertTrainingData(db_manager.GetConnection(),
                                         training_data)) {
          std::cout << "Successfully inserted data from " << json_path
                    << std::endl;
          success_count++;
        } else {
          std::cerr << "Failed to insert data from " << json_path << std::endl;
        }
      }
    }
    std::cout << "\nDatabase insertion complete. " << success_count << " of "
              << json_files.size() << " files inserted successfully."
              << std::endl;
    return std::cmp_equal(success_count, json_files.size());
  }

  if (config.action_ == ActionType::Export) {
    std::cout << "Performing report export from database..." << std::endl;

    fs::path db_path = fs::path(config.base_path_) / "workout_logs.sqlite3";
    if (!fs::exists(db_path)) {
      std::cerr << "Error: Database file not found at " << db_path.string()
                << std::endl;
      return false;
    }

    DbManager db_manager(db_path.string());
    if (!db_manager.Open()) {
      return false;
    }

    fs::path output_dir = fs::path(config.base_path_) / "output_file" / "md";

    if (ReportFacade::GenerateReport(db_manager.GetConnection(),
                                     output_dir.string())) {
      std::cout << "\nReport export completed successfully." << std::endl;
      return true;
    }

    std::cerr << "\nReport export failed." << std::endl;
    return false;
  }

  if (config.action_ == ActionType::QueryPR ||
      config.action_ == ActionType::ListExercises ||
      config.action_ == ActionType::QueryCycles ||
      config.action_ == ActionType::QueryVolume) {
    fs::path db_path = fs::path(config.base_path_) / "workout_logs.sqlite3";
    if (!fs::exists(db_path)) {
      std::cerr << "Error: Database file not found at " << db_path.string()
                << std::endl;
      return false;
    }

    DbManager db_manager(db_path.string());
    if (!db_manager.Open()) {
      return false;
    }

    if (config.action_ == ActionType::QueryPR) {
      auto prs = QueryFacade::QueryAllPRs(db_manager.GetConnection());
      if (prs.empty()) {
        std::cout << "No PR data found." << std::endl;
        return true;
      }
      std::cout << "\n--- Personal Records ---" << std::endl;
      for (const auto& pr : prs) {
        std::cout << pr.exercise_name << ": " << pr.max_weight << "kg x "
                  << pr.reps << " (Date: " << pr.date << ")";
        if (pr.reps > 1) {
          std::cout << " [Est. 1RM: Epley " << std::fixed << std::setprecision(1) << pr.estimated_1rm_epley 
                    << "kg, Brzycki " << pr.estimated_1rm_brzycki << "kg]";
        }
        std::cout << std::endl;
      }
      return true;
    }

    if (config.action_ == ActionType::ListExercises) {
      auto exercises = QueryFacade::GetExercisesByType(
          db_manager.GetConnection(), config.type_filter_);
      if (exercises.empty()) {
        std::cout << "No exercises found"
                  << (config.type_filter_.empty()
                          ? "."
                          : " for type: " + config.type_filter_)
                  << std::endl;
        return true;
      }
      std::cout << "\n--- Exercises ---" << std::endl;
      for (const auto& info : exercises) {
        std::cout << "- " << info.name << " [" << info.type << "]" << std::endl;
      }
      return true;
    }

    if (config.action_ == ActionType::QueryCycles) {
      auto cycles = QueryFacade::GetAllCycles(db_manager.GetConnection());
      if (cycles.empty()) {
        std::cout << "No training cycles found." << std::endl;
        return true;
      }
      std::cout << "\n--- Training Cycles ---" << std::endl;
      for (const auto& cycle : cycles) {
        std::cout << "Cycle: " << cycle.cycle_id << " [" << cycle.type << "] "
                  << "Duration: " << cycle.total_days << " days (" << cycle.start_date
                  << " to " << cycle.end_date << ")" << std::endl;
      }
      return true;
    }

    if (config.action_ == ActionType::QueryVolume) {
      auto stats_opt = QueryFacade::GetVolumeStats(
          db_manager.GetConnection(), config.cycle_id_filter_, config.type_filter_);
      
      if (!stats_opt.has_value()) {
        std::cout << "No data found for Cycle: " << config.cycle_id_filter_
                  << ", Type: " << config.type_filter_ << std::endl;
        return true;
      }

      const auto& stats = stats_opt.value();
      double avg_daily_vol = (stats.total_days > 0) ? (stats.total_volume / stats.total_days) : 0.0;
      double frequency = (stats.total_days > 0) ? (static_cast<double>(stats.session_count) / (stats.total_days / 7.0)) : 0.0;
      double density = (stats.total_days > 0) ? (static_cast<double>(stats.total_sets) / stats.total_days) : 0.0;

      auto get_percent = [&](double vol) {
        return (stats.total_volume > 0) ? (vol / stats.total_volume * 100.0) : 0.0;
      };

      std::cout << "\n--- Advanced Kinematics Analysis ---" << std::endl;
      std::cout << "Cycle:           " << stats.cycle_id << std::endl;
      std::cout << "Type:            " << stats.exercise_type << std::endl;
      std::cout << "------------------------------------" << std::endl;
      std::cout << "Total Volume:    " << std::fixed << std::setprecision(1) << stats.total_volume << "kg" << std::endl;
      std::cout << "Avg Intensity:   " << stats.average_intensity << "kg/rep" << std::endl;
      std::cout << "Avg Daily Vol:   " << avg_daily_vol << "kg/day" << std::endl;
      std::cout << "------------------------------------" << std::endl;
      std::cout << "Sessions:        " << stats.session_count << std::endl;
      std::cout << "Frequency:       " << frequency << " sessions/week" << std::endl;
      std::cout << "Density:         " << density << " sets/day" << std::endl;
      std::cout << "------------------------------------" << std::endl;
      std::cout << "Intensity Distribution:" << std::endl;
      std::cout << "  Power (1-5):   " << get_percent(stats.vol_power) << "%" << std::endl;
      std::cout << "  Hyper (6-12):  " << get_percent(stats.vol_hypertrophy) << "%" << std::endl;
      std::cout << "  Endure (13+):  " << get_percent(stats.vol_endurance) << "%" << std::endl;
      
      return true;
    }
  }

  return false;
}

auto DatabaseHandler::InsertData(const std::vector<DailyData>& data,
                                 const AppConfig& config) -> bool {
  std::cout << "Performing database insertion..." << std::endl;

  fs::path db_path = fs::path(config.base_path_) / "workout_logs.sqlite3";
  DbManager db_manager(db_path.string());

  if (!db_manager.Open()) {
    std::cerr << "Failed to open database." << std::endl;
    return false;
  }

  if (DbFacade::InsertTrainingData(db_manager.GetConnection(), data)) {
    std::cout << "Successfully inserted data." << std::endl;
    return true;
  }

  std::cerr << "Failed to insert data." << std::endl;
  return false;
}