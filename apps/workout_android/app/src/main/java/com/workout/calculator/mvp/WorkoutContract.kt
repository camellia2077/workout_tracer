package com.workout.calculator.mvp

import com.workout.calculator.core.ExerciseRecord
import com.workout.calculator.core.PersonalRecord
import kotlinx.coroutines.flow.StateFlow

enum class WorkoutTab {
    Insert,
    Query,
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

data class WorkoutUiState(
    val isLoading: Boolean = false,
    val statusText: String = "",
    val selectedTab: WorkoutTab = WorkoutTab.Insert,
    val themeMode: ThemeMode = ThemeMode.Light,
    val accentColor: AccentColor = AccentColor.Blue,
    val exercises: List<ExerciseRecord> = emptyList(),
    val personalRecords: List<PersonalRecord> = emptyList(),
    val expandedExerciseName: String? = null,
    val expandedExerciseMarkdown: String = "",
    val expandedPrKey: String? = null,
    val expandedPrMarkdown: String = "",
)

interface WorkoutContract {
    interface Presenter {
        val state: StateFlow<WorkoutUiState>
        fun onTabSelected(tab: WorkoutTab)
        fun onThemeModeSelected(mode: ThemeMode)
        fun onAccentColorSelected(color: AccentColor)
        fun onIngestTestDataClick()
        fun onQueryClick()
        fun onExerciseCardClick(exercise: ExerciseRecord)
        fun onPrCardClick(record: PersonalRecord)
        fun dispose()
    }
}
