package com.workout.calculator.mvp

import android.content.Context
import android.net.Uri
import androidx.documentfile.provider.DocumentFile
import com.workout.calculator.core.CorePayloadParser
import com.workout.calculator.core.CoreStatus
import com.workout.calculator.core.DisplayUnit
import com.workout.calculator.core.ImportFailureDetailRecord
import com.workout.calculator.core.NativeResult
import com.workout.calculator.core.WorkoutNativeBridge
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.io.IOException
import java.util.Locale

class NativeWorkoutRepository(
    private val appContext: Context,
) : WorkoutRepository {

    private val workingDir: File by lazy {
        File(appContext.filesDir, "mvp_core").also { it.mkdirs() }
    }

    private val recordsLiveDir: File
        get() = File(workingDir, "records_live")

    private val recordsStagingDir: File
        get() = File(workingDir, "records_staging")

    private val configDir: File
        get() = File(workingDir, "config")

    private val recordsBackupDir: File
        get() = File(workingDir, "records_backup")

    private val archiveTempDir: File
        get() = File(workingDir, "archive_tmp")

    private val outputDbDir: File
        get() = File(workingDir, "output/db")

    override suspend fun importFolder(folderUri: String): ImportFolderResult =
        withContext(Dispatchers.IO) {
            val treeUri = runCatching { Uri.parse(folderUri) }.getOrNull()
                ?: return@withContext invalidImportResult(
                    message = "invalid folder uri",
                )
            val root = DocumentFile.fromTreeUri(appContext, treeUri)
                ?: return@withContext invalidImportResult(
                    message = "unable to access selected folder",
                )

            val copyFailures = mutableListOf<String>()
            prepareCleanDirectory(recordsStagingDir)
            val copiedTxtCount = copyTxtFilesRecursively(
                node = root,
                relativePath = "",
                targetRoot = recordsStagingDir,
                failures = copyFailures,
            )

            if (copiedTxtCount == 0) {
                cleanupStagingRecords()
                val copyFailureDetails = copyFailuresToDetails(copyFailures)
                return@withContext ImportFolderResult(
                    copiedTxtCount = 0,
                    processedTxtCount = 0,
                    importedTxtCount = 0,
                    failedFiles = copyFailures.ifEmpty { listOf("No .txt files found.") },
                    failedDetails = copyFailureDetails,
                    statusCode = CoreStatus.INVALID_ARGS,
                    message = "no txt files found",
                )
            }

            val mapping = ensureDefaultMapping()
            val nativeResult = WorkoutNativeBridge.rebuildFromLogs(
                logsPath = recordsStagingDir.absolutePath,
                mappingPath = mapping.absolutePath,
                basePath = workingDir.absolutePath,
            )
            val nativeSummary = CorePayloadParser.parseImportSummary(nativeResult.payloadJson)
            val copyFailureDetails = copyFailuresToDetails(copyFailures)
            val combinedFailures = copyFailures + nativeSummary.failedFiles
            val combinedFailureDetails = copyFailureDetails + nativeSummary.failedDetails
            val effectiveStatus = when {
                nativeSummary.importedTxtCount > 0 && combinedFailures.isNotEmpty() ->
                    CoreStatus.PROCESSING_ERROR

                else -> nativeResult.statusCode
            }
            val effectiveMessage = when {
                nativeResult.message.isNotBlank() -> nativeResult.message
                nativeSummary.importedTxtCount > 0 && combinedFailures.isNotEmpty() ->
                    "partial import completed"

                nativeSummary.importedTxtCount > 0 -> "import completed"
                else -> "import failed"
            }

            if (nativeSummary.importedTxtCount > 0) {
                if (!promoteStagingRecords()) {
                    cleanupStagingRecords()
                    return@withContext ImportFolderResult(
                        copiedTxtCount = copiedTxtCount,
                        processedTxtCount = nativeSummary.processedTxtCount,
                        importedTxtCount = nativeSummary.importedTxtCount,
                        failedFiles = combinedFailures,
                        failedDetails = combinedFailureDetails + ImportFailureDetailRecord(
                            file = "<promote_staging>",
                            stage = "android_promote",
                            diagnostics = listOf("failed to promote imported txt snapshot"),
                        ),
                        statusCode = CoreStatus.PROCESSING_ERROR,
                        message = "failed to promote imported txt snapshot",
                    )
                }
            } else {
                cleanupStagingRecords()
            }

            ImportFolderResult(
                copiedTxtCount = copiedTxtCount,
                processedTxtCount = nativeSummary.processedTxtCount,
                importedTxtCount = nativeSummary.importedTxtCount,
                failedFiles = combinedFailures,
                failedDetails = combinedFailureDetails,
                statusCode = effectiveStatus,
                message = effectiveMessage,
            )
        }

    override suspend fun exportArchive(targetUri: String): ExportArchiveResult =
        withContext(Dispatchers.IO) {
            val uri = runCatching { Uri.parse(targetUri) }.getOrNull()
                ?: return@withContext ExportArchiveResult(
                    recordsCount = 0,
                    jsonCount = 0,
                    statusCode = CoreStatus.INVALID_ARGS,
                    message = "invalid archive target uri",
                )

            prepareCleanDirectory(archiveTempDir)
            val tempArchiveFile = File(archiveTempDir, "workout_backup.zip")
            val nativeResult = WorkoutNativeBridge.exportArchive(
                basePath = workingDir.absolutePath,
                archiveOutputPath = tempArchiveFile.absolutePath,
            )
            val summary = CorePayloadParser.parseArchiveExportSummary(nativeResult.payloadJson)
            if (nativeResult.statusCode != CoreStatus.SUCCESS || !tempArchiveFile.exists()) {
                cleanupArchiveTemp()
                return@withContext ExportArchiveResult(
                    recordsCount = summary.recordsCount,
                    jsonCount = summary.jsonCount,
                    statusCode = nativeResult.statusCode,
                    message = nativeResult.message.ifBlank { "archive export failed" },
                )
            }

            val copySucceeded = runCatching {
                appContext.contentResolver.openOutputStream(uri).use { output ->
                    if (output == null) {
                        error("unable to open archive target output stream")
                    }
                    tempArchiveFile.inputStream().use { input ->
                        input.copyTo(output)
                    }
                }
            }.isSuccess

            cleanupArchiveTemp()
            if (!copySucceeded) {
                return@withContext ExportArchiveResult(
                    recordsCount = summary.recordsCount,
                    jsonCount = summary.jsonCount,
                    statusCode = CoreStatus.PROCESSING_ERROR,
                    message = "failed to write archive to selected destination",
                )
            }

            ExportArchiveResult(
                recordsCount = summary.recordsCount,
                jsonCount = summary.jsonCount,
                statusCode = nativeResult.statusCode,
                message = nativeResult.message.ifBlank { "archive export completed" },
            )
        }

    override suspend fun importArchive(archiveUri: String): ImportArchiveResult =
        withContext(Dispatchers.IO) {
            val uri = runCatching { Uri.parse(archiveUri) }.getOrNull()
                ?: return@withContext ImportArchiveResult(
                    processedTxtCount = 0,
                    importedTxtCount = 0,
                    failedFiles = emptyList(),
                    statusCode = CoreStatus.INVALID_ARGS,
                    message = "invalid archive uri",
                )

            prepareCleanDirectory(archiveTempDir)
            val tempArchiveFile = File(archiveTempDir, "import_bundle.zip")
            val stagedArchive = runCatching {
                appContext.contentResolver.openInputStream(uri).use { input ->
                    if (input == null) {
                        error("unable to open archive input stream")
                    }
                    tempArchiveFile.outputStream().use { output ->
                        input.copyTo(output)
                    }
                }
            }.isSuccess
            if (!stagedArchive) {
                cleanupArchiveTemp()
                return@withContext ImportArchiveResult(
                    processedTxtCount = 0,
                    importedTxtCount = 0,
                    failedFiles = emptyList(),
                    statusCode = CoreStatus.PROCESSING_ERROR,
                    message = "failed to copy selected archive",
                )
            }

            val nativeResult = WorkoutNativeBridge.importArchive(
                basePath = workingDir.absolutePath,
                archivePath = tempArchiveFile.absolutePath,
            )
            val summary = CorePayloadParser.parseImportSummary(nativeResult.payloadJson)
            cleanupArchiveTemp()
            ImportArchiveResult(
                processedTxtCount = summary.processedTxtCount,
                importedTxtCount = summary.importedTxtCount,
                failedFiles = summary.failedFiles,
                statusCode = nativeResult.statusCode,
                message = nativeResult.message.ifBlank {
                    when {
                        summary.importedTxtCount > 0 && summary.failedFiles.isNotEmpty() ->
                            "partial archive import completed"

                        summary.importedTxtCount > 0 -> "archive import completed"
                        else -> "archive import failed"
                    }
                },
            )
        }

    override suspend fun clearDatabase(): NativeResult = withContext(Dispatchers.IO) {
        runCatching {
            if (!outputDbDir.exists()) {
                return@runCatching NativeResult(
                    statusCode = CoreStatus.SUCCESS,
                    message = "database already empty",
                    payloadJson = "{}",
                )
            }

            if (!outputDbDir.deleteRecursively()) {
                return@runCatching NativeResult(
                    statusCode = CoreStatus.PROCESSING_ERROR,
                    message = "failed to clear database files",
                    payloadJson = "{}",
                )
            }

            NativeResult(
                statusCode = CoreStatus.SUCCESS,
                message = "database cleared",
                payloadJson = "{}",
            )
        }.getOrElse { throwable ->
            NativeResult(
                statusCode = CoreStatus.PROCESSING_ERROR,
                message = throwable.message ?: "failed to clear database files",
                payloadJson = "{}",
            )
        }
    }

    override suspend fun clearTxtFiles(): NativeResult = withContext(Dispatchers.IO) {
        runCatching {
            val targets = listOf(recordsLiveDir, recordsStagingDir, recordsBackupDir)
            var hasAnyTxtSnapshot = false
            targets.forEach { target ->
                if (!target.exists()) {
                    return@forEach
                }
                hasAnyTxtSnapshot = true
                if (!target.deleteRecursively()) {
                    return@runCatching NativeResult(
                        statusCode = CoreStatus.PROCESSING_ERROR,
                        message = "failed to clear txt files",
                        payloadJson = "{}",
                    )
                }
            }

            NativeResult(
                statusCode = CoreStatus.SUCCESS,
                message = if (hasAnyTxtSnapshot) {
                    "txt files cleared"
                } else {
                    "txt files already empty"
                },
                payloadJson = "{}",
            )
        }.getOrElse { throwable ->
            NativeResult(
                statusCode = CoreStatus.PROCESSING_ERROR,
                message = throwable.message ?: "failed to clear txt files",
                payloadJson = "{}",
            )
        }
    }

    override suspend fun listExercises(): NativeResult = withContext(Dispatchers.IO) {
        WorkoutNativeBridge.listExercises(
            basePath = workingDir.absolutePath,
            typeFilter = "",
        )
    }

    override suspend fun queryPrs(): NativeResult = withContext(Dispatchers.IO) {
        WorkoutNativeBridge.queryPrs(basePath = workingDir.absolutePath)
    }

    override suspend fun queryCycles(): NativeResult = withContext(Dispatchers.IO) {
        WorkoutNativeBridge.queryCycles(basePath = workingDir.absolutePath)
    }

    override suspend fun queryCycleVolumes(cycleId: String): NativeResult =
        withContext(Dispatchers.IO) {
            WorkoutNativeBridge.queryCycleVolumes(
                basePath = workingDir.absolutePath,
                cycleId = cycleId,
            )
        }

    override suspend fun queryCycleTypeReport(
        cycleId: String,
        exerciseType: String,
        displayUnit: DisplayUnit,
    ): NativeResult = withContext(Dispatchers.IO) {
        WorkoutNativeBridge.queryCycleTypeReport(
            basePath = workingDir.absolutePath,
            cycleId = cycleId,
            exerciseType = exerciseType,
            displayUnit = displayUnit.toCoreValue(),
        )
    }

    private fun ensureDefaultMapping(): File {
        val target = File(configDir, "mapping.toml")
        target.parentFile?.mkdirs()
        appContext.assets.open("mapping.toml").use { input ->
            target.outputStream().use { output ->
                input.copyTo(output)
            }
        }
        return target
    }

    private fun copyTxtFilesRecursively(
        node: DocumentFile,
        relativePath: String,
        targetRoot: File,
        failures: MutableList<String>,
    ): Int {
        if (node.isFile) {
            val name = node.name
            if (name.isNullOrBlank()) {
                failures += appendFailure(relativePath, "<unnamed file>")
                return 0
            }
            if (!name.lowercase(Locale.ROOT).endsWith(".txt")) {
                return 0
            }

            return try {
                val target = File(targetRoot, relativePath)
                target.parentFile?.mkdirs()
                appContext.contentResolver.openInputStream(node.uri).use { input ->
                    if (input == null) {
                        failures += appendFailure(relativePath, "unable to open input stream")
                        return 0
                    }
                    target.outputStream().use { output ->
                        input.copyTo(output)
                    }
                }
                1
            } catch (exception: Exception) {
                failures += appendFailure(relativePath, exception.message ?: "copy failed")
                0
            }
        }

        if (!node.isDirectory) {
            return 0
        }

        return runCatching { node.listFiles().toList() }
            .getOrElse {
                failures += appendFailure(
                    relativePath.ifBlank { node.name ?: "<folder>" },
                    it.message ?: "failed to list directory",
                )
                emptyList()
            }
            .sumOf { child ->
                val childName = child.name ?: return@sumOf 0
                val childRelativePath = if (relativePath.isBlank()) {
                    childName
                } else {
                    "$relativePath/$childName"
                }
                copyTxtFilesRecursively(
                    node = child,
                    relativePath = childRelativePath,
                    targetRoot = targetRoot,
                    failures = failures,
                )
            }
    }

    private fun prepareCleanDirectory(directory: File) {
        if (directory.exists()) {
            directory.deleteRecursively()
        }
        directory.mkdirs()
    }

    private fun cleanupStagingRecords() {
        if (recordsStagingDir.exists()) {
            recordsStagingDir.deleteRecursively()
        }
    }

    private fun cleanupArchiveTemp() {
        if (archiveTempDir.exists()) {
            archiveTempDir.deleteRecursively()
        }
    }

    private fun promoteStagingRecords(): Boolean {
        if (recordsBackupDir.exists()) {
            recordsBackupDir.deleteRecursively()
        }
        val hadLiveSnapshot = recordsLiveDir.exists()
        if (hadLiveSnapshot && !recordsLiveDir.renameTo(recordsBackupDir)) {
            return false
        }

        if (recordsStagingDir.renameTo(recordsLiveDir)) {
            if (recordsBackupDir.exists()) {
                recordsBackupDir.deleteRecursively()
            }
            return true
        }

        val promoted = runCatching {
            copyDirectoryRecursively(recordsStagingDir, recordsLiveDir)
            recordsStagingDir.deleteRecursively()
            true
        }.getOrDefault(false)
        if (promoted) {
            if (recordsBackupDir.exists()) {
                recordsBackupDir.deleteRecursively()
            }
            return true
        }

        if (recordsLiveDir.exists()) {
            recordsLiveDir.deleteRecursively()
        }
        if (hadLiveSnapshot && recordsBackupDir.exists()) {
            recordsBackupDir.renameTo(recordsLiveDir)
        }
        return false
    }

    private fun copyDirectoryRecursively(source: File, target: File) {
        if (!source.exists()) {
            throw IOException("source directory missing: ${source.absolutePath}")
        }
        source.walkTopDown().forEach { entry ->
            val relative = entry.relativeTo(source)
            val destination = File(target, relative.path)
            if (entry.isDirectory) {
                destination.mkdirs()
            } else {
                destination.parentFile?.mkdirs()
                entry.inputStream().use { input ->
                    destination.outputStream().use { output ->
                        input.copyTo(output)
                    }
                }
            }
        }
    }

    private fun appendFailure(path: String, reason: String): String {
        return if (path.isBlank()) {
            reason
        } else {
            "$path ($reason)"
        }
    }

    private fun invalidImportResult(message: String): ImportFolderResult {
        return ImportFolderResult(
            copiedTxtCount = 0,
            processedTxtCount = 0,
            importedTxtCount = 0,
            failedFiles = emptyList(),
            failedDetails = emptyList(),
            statusCode = CoreStatus.INVALID_ARGS,
            message = message,
        )
    }

    private fun copyFailuresToDetails(copyFailures: List<String>): List<ImportFailureDetailRecord> {
        if (copyFailures.isEmpty()) {
            return emptyList()
        }
        return copyFailures.map { failure ->
            ImportFailureDetailRecord(
                file = failure.substringBefore(" ("),
                stage = "android_copy",
                diagnostics = listOf(failure),
            )
        }
    }

    private fun DisplayUnit.toCoreValue(): String {
        return when (this) {
            DisplayUnit.Original -> "original"
            DisplayUnit.Kg -> "kg"
            DisplayUnit.Lb -> "lb"
        }
    }
}
