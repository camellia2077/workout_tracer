#ifndef APPLICATION_INTERFACES_I_ARCHIVE_SERVICE_HPP_
#define APPLICATION_INTERFACES_I_ARCHIVE_SERVICE_HPP_

#include "application/interfaces/archive_models.hpp"
#include "core/application/use_case_result.hpp"

class IArchiveService {
 public:
  virtual ~IArchiveService() = default;

  [[nodiscard]] virtual auto ExportArchive(const ArchiveExportRequest& request)
      -> UseCaseResult<ArchiveExportSummary> = 0;

  [[nodiscard]] virtual auto ImportArchive(const ArchiveImportRequest& request)
      -> UseCaseResult<ArchiveImportSummary> = 0;
};

#endif  // APPLICATION_INTERFACES_I_ARCHIVE_SERVICE_HPP_
