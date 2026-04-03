#ifndef APPLICATION_INTERFACES_ARCHIVE_MODELS_HPP_
#define APPLICATION_INTERFACES_ARCHIVE_MODELS_HPP_

#include <string>
#include <vector>

struct ArchiveExportRequest {
  std::string base_path_;
  std::string archive_output_path_;
};

struct ArchiveImportRequest {
  std::string base_path_;
  std::string archive_path_;
};

struct ArchiveExportSummary {
  int records_count_ = 0;
  int json_count_ = 0;
  std::string archive_output_path_;
};

struct ArchiveImportSummary {
  int processed_txt_count_ = 0;
  int imported_txt_count_ = 0;
  std::vector<std::string> failed_files_;
};

#endif  // APPLICATION_INTERFACES_ARCHIVE_MODELS_HPP_
