package com.workout.calculator.mvp

import com.workout.calculator.core.ImportFailureDetailRecord

data class ImportFolderResult(
    val copiedTxtCount: Int,
    val processedTxtCount: Int,
    val importedTxtCount: Int,
    val failedFiles: List<String>,
    val failedDetails: List<ImportFailureDetailRecord> = emptyList(),
    val statusCode: Int,
    val message: String,
) {
    val hasImportedData: Boolean
        get() = importedTxtCount > 0
}
