package com.workout.calculator.mvp

import com.workout.calculator.core.CorePayloadParser
import com.workout.calculator.core.CoreStatus
import com.workout.calculator.core.CycleRecord
import com.workout.calculator.core.DisplayUnit
import com.workout.calculator.core.DisplayUnitFormatter
import com.workout.calculator.core.NativeResult
import com.workout.calculator.core.PersonalRecord
import com.workout.calculator.core.isSuccess
import com.workout.calculator.core.toStatusText
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch

class WorkoutPresenter(
    private val repository: WorkoutRepository,
    private val preferenceStorage: UiPreferenceStorage,
) : WorkoutContract.Presenter {

    private val presenterScope = CoroutineScope(SupervisorJob() + Dispatchers.Main.immediate)
    private val mutableState = MutableStateFlow(WorkoutUiState(statusText = "Ready."))

    override val state: StateFlow<WorkoutUiState> = mutableState.asStateFlow()

    init {
        presenterScope.launch {
            val loadedMode = runCatching { preferenceStorage.loadThemeMode() }
                .getOrDefault(ThemeMode.Light)
            val loadedAccentColor = runCatching { preferenceStorage.loadAccentColor() }
                .getOrDefault(AccentColor.Blue)
            val loadedDisplayUnit = runCatching { preferenceStorage.loadDisplayUnit() }
                .getOrDefault(DisplayUnit.Original)
            mutableState.update { current ->
                if (current.themeMode == loadedMode &&
                    current.accentColor == loadedAccentColor &&
                    current.displayUnit == loadedDisplayUnit
                ) {
                    current
                } else {
                    current.copy(
                        themeMode = loadedMode,
                        accentColor = loadedAccentColor,
                        displayUnit = loadedDisplayUnit,
                    )
                }
            }
        }
    }

    override fun onTabSelected(tab: WorkoutTab) {
        mutableState.update { current ->
            if (current.selectedTab == tab) {
                current
            } else {
                current.copy(selectedTab = tab)
            }
        }
    }

    override fun onThemeModeSelected(mode: ThemeMode) {
        mutableState.update { current ->
            if (current.themeMode == mode) {
                current
            } else {
                current.copy(themeMode = mode)
            }
        }
        presenterScope.launch {
            runCatching { preferenceStorage.saveThemeMode(mode) }
        }
    }

    override fun onAccentColorSelected(color: AccentColor) {
        mutableState.update { current ->
            if (current.accentColor == color) {
                current
            } else {
                current.copy(accentColor = color)
            }
        }
        presenterScope.launch {
            runCatching { preferenceStorage.saveAccentColor(color) }
        }
    }

    override fun onDisplayUnitSelected(displayUnit: DisplayUnit) {
        val previous = mutableState.value
        if (previous.displayUnit == displayUnit) {
            return
        }
        mutableState.update { current ->
            current.copy(
                displayUnit = displayUnit,
                expandedPrMarkdown = rebuildExpandedPrMarkdown(current, displayUnit),
                currentReportMarkdown = "",
                isReportLoading = false,
            )
        }

        val selectedMonth = mutableState.value.selectedMonth
        val selectedType = mutableState.value.selectedType
        presenterScope.launch {
            runCatching { preferenceStorage.saveDisplayUnit(displayUnit) }
            if (selectedMonth != null) {
                fetchReportFor(month = selectedMonth, type = selectedType)
            }
        }
    }

    override fun onImportFolderSelected(folderUri: String) {
        presenterScope.launch {
            mutableState.update {
                it.copy(
                    isLoading = true,
                    isReportLoading = false,
                    statusText = "Importing txt files...",
                )
            }

            val importResult = repository.importFolder(folderUri)
            val importStatus = buildImportStatusLine(importResult)
            if (!importResult.hasImportedData) {
                mutableState.update {
                    it.copy(
                        isLoading = false,
                        isReportLoading = false,
                        statusText = importStatus,
                    )
                }
                return@launch
            }

            refreshQuerySnapshot(prefixStatus = importStatus)
        }
    }

    override fun onQueryClick() {
        presenterScope.launch {
            mutableState.update {
                it.copy(
                    isLoading = true,
                    isReportLoading = false,
                    statusText = "Querying from sqlite...",
                )
            }
            refreshQuerySnapshot(prefixStatus = null)
        }
    }

    override fun onMonthSelected(month: String) {
        presenterScope.launch {
            val current = mutableState.value
            if (!current.monthOptions.contains(month)) {
                return@launch
            }
            mutableState.update {
                it.copy(
                    selectedMonth = month,
                    selectedType = QueryType.Squat,
                    currentReportMarkdown = "",
                    isReportLoading = false,
                )
            }
            fetchReportFor(month = month, type = QueryType.Squat)
        }
    }

    override fun onTypeSelected(type: QueryType) {
        presenterScope.launch {
            val month = mutableState.value.selectedMonth
            mutableState.update {
                it.copy(
                    selectedType = type,
                    currentReportMarkdown = "",
                    isReportLoading = false,
                )
            }
            if (month == null) {
                mutableState.update {
                    it.copy(statusText = "Select a month first.")
                }
                return@launch
            }
            fetchReportFor(month = month, type = type)
        }
    }

    override fun onImportArchiveSelected(archiveUri: String) {
        presenterScope.launch {
            mutableState.update {
                it.copy(
                    isLoading = true,
                    isReportLoading = false,
                    statusText = "Importing archive...",
                )
            }

            val importResult = repository.importArchive(archiveUri)
            val importStatus = buildArchiveImportStatusLine(importResult)
            if (!importResult.hasImportedData) {
                mutableState.update {
                    it.copy(
                        isLoading = false,
                        isReportLoading = false,
                        statusText = importStatus,
                    )
                }
                return@launch
            }

            refreshQuerySnapshot(prefixStatus = importStatus)
        }
    }

    override fun onExportArchiveSelected(targetUri: String) {
        presenterScope.launch {
            mutableState.update {
                it.copy(
                    isLoading = true,
                    isReportLoading = false,
                    statusText = "Exporting archive...",
                )
            }

            val exportResult = repository.exportArchive(targetUri)
            mutableState.update {
                it.copy(
                    isLoading = false,
                    isReportLoading = false,
                    statusText = buildArchiveExportStatusLine(exportResult),
                )
            }
        }
    }

    override fun onClearDatabaseClick() {
        presenterScope.launch {
            mutableState.update {
                it.copy(
                    isLoading = true,
                    isReportLoading = false,
                    statusText = "Clearing database...",
                )
            }

            val clearResult = repository.clearDatabase()
            val clearStatus = buildActionStatusLine(action = "Clear DB", result = clearResult)
            if (clearResult.isSuccess()) {
                refreshQuerySnapshot(prefixStatus = clearStatus)
            } else {
                mutableState.update {
                    it.copy(
                        isLoading = false,
                        isReportLoading = false,
                        statusText = clearStatus,
                    )
                }
            }
        }
    }

    override fun onClearTxtFilesClick() {
        presenterScope.launch {
            mutableState.update {
                it.copy(
                    isLoading = true,
                    isReportLoading = false,
                    statusText = "Clearing txt files...",
                )
            }

            val clearResult = repository.clearTxtFiles()
            val clearStatus = buildActionStatusLine(action = "Clear TXT", result = clearResult)
            if (clearResult.isSuccess()) {
                refreshQuerySnapshot(prefixStatus = clearStatus)
            } else {
                mutableState.update {
                    it.copy(
                        isLoading = false,
                        isReportLoading = false,
                        statusText = clearStatus,
                    )
                }
            }
        }
    }

    override fun onPrCardClick(record: PersonalRecord) {
        val prKey = buildPrKey(record)
        mutableState.update { current ->
            if (current.expandedPrKey == prKey) {
                current.copy(
                    expandedPrKey = null,
                    expandedPrMarkdown = "",
                )
            } else {
                current.copy(
                    expandedPrKey = prKey,
                    expandedPrMarkdown = buildPrMarkdown(record, current.displayUnit),
                )
            }
        }
    }

    override fun dispose() {
        presenterScope.cancel()
    }

    private suspend fun refreshQuerySnapshot(prefixStatus: String?) {
        val queryState = querySnapshotStatus()
        val monthOptions = buildMonthOptions(queryState.cycles)
        val selectedMonth = monthOptions.firstOrNull()

        mutableState.update {
            it.copy(
                isLoading = false,
                statusText = buildQueryStatusLine(
                    prefixStatus = prefixStatus,
                    queryStatus = queryState.statusLine,
                ),
                personalRecords = queryState.prs,
                cycles = queryState.cycles,
                expandedPrKey = null,
                expandedPrMarkdown = "",
                monthOptions = monthOptions,
                selectedMonth = selectedMonth,
                selectedType = QueryType.Squat,
                currentReportMarkdown = "",
                isReportLoading = false,
            )
        }

        if (selectedMonth != null) {
            fetchReportFor(month = selectedMonth, type = QueryType.Squat)
        }
    }

    private suspend fun fetchReportFor(month: String, type: QueryType) {
        mutableState.update { current ->
            if (current.selectedMonth != month || current.selectedType != type) {
                current
            } else {
                current.copy(
                    isReportLoading = true,
                    currentReportMarkdown = "",
                    statusText = "Loading report preview for $month/${type.coreValue}...",
                )
            }
        }

        val displayUnit = mutableState.value.displayUnit
        val reportResult = repository.queryCycleTypeReport(
            cycleId = month,
            exerciseType = type.coreValue,
            displayUnit = displayUnit,
        )
        val reportRecord = if (reportResult.isSuccess()) {
            CorePayloadParser.parseCycleTypeReport(reportResult.payloadJson)
        } else {
            null
        }

        mutableState.update { current ->
            if (current.selectedMonth != month || current.selectedType != type) {
                current
            } else {
                current.copy(
                    isReportLoading = false,
                    currentReportMarkdown = reportRecord?.markdown.orEmpty(),
                    statusText = buildReportStatusLine(
                        cycleId = month,
                        exerciseType = type.coreValue,
                        reportStatus = reportResult.toStatusText(prefix = "Report"),
                    ),
                )
            }
        }
    }

    private fun buildPrKey(record: PersonalRecord): String {
        return buildPrKey(
            exerciseName = record.exerciseName,
            date = record.date,
            reps = record.reps,
            maxWeightKg = record.maxWeightKg,
            originalUnit = record.originalUnit,
            originalWeightValue = record.originalWeightValue,
        )
    }

    private fun buildPrMarkdown(record: PersonalRecord, displayUnit: DisplayUnit): String {
        val maxWeight = DisplayUnitFormatter.formatDetailWeight(
            weightKg = record.maxWeightKg,
            originalUnit = record.originalUnit,
            originalWeightValue = record.originalWeightValue,
            displayUnit = displayUnit,
        )
        val oneRmEpley = DisplayUnitFormatter.formatOneRmOrDash(
            oneRmKg = record.estimatedOneRmEpleyKg,
            referenceWeightKg = record.maxWeightKg,
            originalUnit = record.originalUnit,
            displayUnit = displayUnit,
        )
        val oneRmBrzycki = DisplayUnitFormatter.formatOneRmOrDash(
            oneRmKg = record.estimatedOneRmBrzyckiKg,
            referenceWeightKg = record.maxWeightKg,
            originalUnit = record.originalUnit,
            displayUnit = displayUnit,
        )
        return """
            # Personal Record
            
            ## ${record.exerciseName}
            - Date: `${record.date}`
            - Max Weight: **$maxWeight**
            - Reps: **${record.reps}**
            - 1RM (Epley): **$oneRmEpley**
            - 1RM (Brzycki): **$oneRmBrzycki**
        """.trimIndent()
    }

    private suspend fun querySnapshotStatus(): QueryState {
        val prsResult = repository.queryPrs()
        val cyclesResult = repository.queryCycles()

        val prs = if (prsResult.isSuccess()) {
            CorePayloadParser.parsePrs(prsResult.payloadJson)
        } else {
            emptyList()
        }
        val cycles = if (cyclesResult.isSuccess()) {
            CorePayloadParser.parseCycles(cyclesResult.payloadJson)
        } else {
            emptyList()
        }

        val statusLine = buildString {
            append(prsResult.toStatusText(prefix = "PR"))
            append('\n')
            append(cyclesResult.toStatusText(prefix = "Cycles"))
            if (prsResult.statusCode == CoreStatus.FILE_NOT_FOUND ||
                cyclesResult.statusCode == CoreStatus.FILE_NOT_FOUND
            ) {
                append('\n')
                append("Hint: import a txt folder or archive first.")
            }
        }
        return QueryState(
            statusLine = statusLine,
            prs = prs,
            cycles = cycles,
        )
    }

    private fun buildMonthOptions(cycles: List<CycleRecord>): List<String> {
        return cycles.map { it.cycleId }
            .filter(::isYearMonth)
            .distinct()
            .sortedDescending()
    }

    private fun isYearMonth(value: String): Boolean {
        return value.length == 7 &&
            value[4] == '-' &&
            value.substring(0, 4).all(Char::isDigit) &&
            value.substring(5, 7).all(Char::isDigit)
    }

    private fun buildPrKey(
        exerciseName: String,
        date: String,
        reps: Int,
        maxWeightKg: Double,
        originalUnit: String,
        originalWeightValue: Double,
    ): String {
        return "$exerciseName|$date|$reps|$maxWeightKg|$originalUnit|$originalWeightValue"
    }

    private fun rebuildExpandedPrMarkdown(
        current: WorkoutUiState,
        displayUnit: DisplayUnit,
    ): String {
        val expandedKey = current.expandedPrKey ?: return ""
        val expandedRecord = current.personalRecords.firstOrNull { buildPrKey(it) == expandedKey }
            ?: return ""
        return buildPrMarkdown(expandedRecord, displayUnit)
    }

    private fun buildImportStatusLine(importResult: ImportFolderResult): String {
        return buildString {
            append("Import: code=")
            append(importResult.statusCode)
            append(", message=")
            append(importResult.message.ifBlank { "ok" })
            append('\n')
            append("Copied TXT: ")
            append(importResult.copiedTxtCount)
            append('\n')
            append("Processed TXT: ")
            append(importResult.processedTxtCount)
            append('\n')
            append("Imported TXT: ")
            append(importResult.importedTxtCount)
            if (importResult.failedFiles.isNotEmpty()) {
                append('\n')
                append("Failed Files:")
                importResult.failedFiles.forEach { failedFile ->
                    append('\n')
                    append("- ")
                    append(failedFile)
                }
            }
            if (importResult.failedDetails.isNotEmpty()) {
                append('\n')
                append("Failure Details:")
                importResult.failedDetails.forEach { detail ->
                    append('\n')
                    append("- ")
                    append(detail.file.ifBlank { "<unknown>" })
                    if (detail.stage.isNotBlank()) {
                        append(" [")
                        append(detail.stage)
                        append(']')
                    }
                    val limitedDiagnostics = detail.diagnostics.take(6)
                    limitedDiagnostics.forEach { diagnostic ->
                        append('\n')
                        append("  - ")
                        append(diagnostic)
                    }
                    if (detail.diagnostics.size > limitedDiagnostics.size) {
                        append('\n')
                        append("  - ...")
                    }
                }
            }
        }
    }

    private fun buildArchiveImportStatusLine(importResult: ImportArchiveResult): String {
        return buildString {
            append("Archive Import: code=")
            append(importResult.statusCode)
            append(", message=")
            append(importResult.message.ifBlank { "ok" })
            append('\n')
            append("Processed TXT: ")
            append(importResult.processedTxtCount)
            append('\n')
            append("Imported TXT: ")
            append(importResult.importedTxtCount)
            if (importResult.failedFiles.isNotEmpty()) {
                append('\n')
                append("Failed Files:")
                importResult.failedFiles.forEach { failedFile ->
                    append('\n')
                    append("- ")
                    append(failedFile)
                }
            }
        }
    }

    private fun buildArchiveExportStatusLine(exportResult: ExportArchiveResult): String {
        return buildString {
            append("Archive Export: code=")
            append(exportResult.statusCode)
            append(", message=")
            append(exportResult.message.ifBlank { "ok" })
            append('\n')
            append("Records: ")
            append(exportResult.recordsCount)
            append('\n')
            append("JSON: ")
            append(exportResult.jsonCount)
        }
    }

    private fun buildActionStatusLine(action: String, result: NativeResult): String {
        return buildString {
            append(action)
            append(": code=")
            append(result.statusCode)
            append(", message=")
            append(result.message.ifBlank { "ok" })
        }
    }

    private fun buildQueryStatusLine(
        prefixStatus: String?,
        queryStatus: String,
    ): String {
        return if (prefixStatus.isNullOrBlank()) {
            queryStatus
        } else {
            buildString {
                append(prefixStatus)
                append('\n')
                append(queryStatus)
            }
        }
    }

    private fun buildReportStatusLine(
        cycleId: String,
        exerciseType: String,
        reportStatus: String,
    ): String {
        return buildString {
            append("Selected month: ")
            append(cycleId)
            append('\n')
            append("Report preview: ")
            append(exerciseType)
            append('\n')
            append(reportStatus)
        }
    }

    private data class QueryState(
        val statusLine: String,
        val prs: List<PersonalRecord>,
        val cycles: List<CycleRecord>,
    )
}
