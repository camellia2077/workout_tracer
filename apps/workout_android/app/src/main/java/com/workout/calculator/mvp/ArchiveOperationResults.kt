package com.workout.calculator.mvp

data class ExportArchiveResult(
    val recordsCount: Int,
    val jsonCount: Int,
    val statusCode: Int,
    val message: String,
)

data class ImportArchiveResult(
    val processedTxtCount: Int,
    val importedTxtCount: Int,
    val failedFiles: List<String>,
    val statusCode: Int,
    val message: String,
) {
    val hasImportedData: Boolean
        get() = importedTxtCount > 0
}
