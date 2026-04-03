#ifndef INFRASTRUCTURE_ARCHIVE_ZIP_ARCHIVE_SERVICE_HPP_
#define INFRASTRUCTURE_ARCHIVE_ZIP_ARCHIVE_SERVICE_HPP_

#include "application/interfaces/i_archive_service.hpp"
#include "application/interfaces/i_log_parser.hpp"
#include "application/interfaces/i_mapping_provider.hpp"

class ZipArchiveService : public IArchiveService {
 public:
  ZipArchiveService(ILogParser& parser, IMappingProvider& mapping_provider);

  [[nodiscard]] auto ExportArchive(const ArchiveExportRequest& request)
      -> UseCaseResult<ArchiveExportSummary> override;

  [[nodiscard]] auto ImportArchive(const ArchiveImportRequest& request)
      -> UseCaseResult<ArchiveImportSummary> override;

 private:
  ILogParser& parser_;
  IMappingProvider& mapping_provider_;
};

#endif  // INFRASTRUCTURE_ARCHIVE_ZIP_ARCHIVE_SERVICE_HPP_
