package com.workout.calculator.mvp

import com.workout.calculator.core.CycleRecord
import com.workout.calculator.core.DisplayUnit
import com.workout.calculator.core.PersonalRecord
import kotlinx.coroutines.flow.StateFlow

enum class WorkoutTab {
    Data,
    Query,
    Records,
    Config,
}

enum class ThemeMode {
    Light,
    Dark,
    System,
}

enum class AccentColor {
    Red,
    Orange,
    Yellow,
    Green,
    Cyan,
    Blue,
    Purple,
}

enum class QueryType(val coreValue: String) {
    Squat("squat"),
    Pull("pull"),
    Push("push"),
}

data class WorkoutUiState(
    val isLoading: Boolean = false,
    val statusText: String = "",
    val selectedTab: WorkoutTab = WorkoutTab.Data,
    val themeMode: ThemeMode = ThemeMode.Light,
    val accentColor: AccentColor = AccentColor.Blue,
    val displayUnit: DisplayUnit = DisplayUnit.Original,
    val personalRecords: List<PersonalRecord> = emptyList(),
    val cycles: List<CycleRecord> = emptyList(),
    val expandedPrKey: String? = null,
    val expandedPrMarkdown: String = "",
    val monthOptions: List<String> = emptyList(),
    val selectedMonth: String? = null,
    val selectedType: QueryType = QueryType.Squat,
    val currentReportMarkdown: String = "",
    val isReportLoading: Boolean = false,
)

interface WorkoutContract {
    interface Presenter {
        val state: StateFlow<WorkoutUiState>
        fun onTabSelected(tab: WorkoutTab)
        fun onThemeModeSelected(mode: ThemeMode)
        fun onAccentColorSelected(color: AccentColor)
        fun onDisplayUnitSelected(displayUnit: DisplayUnit)
        fun onImportFolderSelected(folderUri: String)
        fun onImportArchiveSelected(archiveUri: String)
        fun onExportArchiveSelected(targetUri: String)
        fun onClearDatabaseClick()
        fun onClearTxtFilesClick()
        fun onQueryClick()
        fun onPrCardClick(record: PersonalRecord)
        fun onMonthSelected(month: String)
        fun onTypeSelected(type: QueryType)
        fun dispose()
    }
}
