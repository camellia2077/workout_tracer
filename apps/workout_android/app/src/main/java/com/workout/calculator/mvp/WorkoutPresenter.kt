package com.workout.calculator.mvp

import com.workout.calculator.core.CorePayloadParser
import com.workout.calculator.core.CoreStatus
import com.workout.calculator.core.ExerciseRecord
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
import java.util.Locale

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
            mutableState.update { current ->
                if (current.themeMode == loadedMode && current.accentColor == loadedAccentColor) {
                    current
                } else {
                    current.copy(
                        themeMode = loadedMode,
                        accentColor = loadedAccentColor,
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

    override fun onIngestTestDataClick() {
        presenterScope.launch {
            mutableState.update {
                it.copy(
                    isLoading = true,
                    statusText = "Ingesting test file...",
                )
            }

            val ingestResult = repository.ingestTestData()
            val status = ingestResult.toStatusText(prefix = "Ingest")
            if (!ingestResult.isSuccess()) {
                mutableState.update {
                    it.copy(
                        isLoading = false,
                        statusText = status,
                    )
                }
                return@launch
            }

            val queryState = querySnapshotStatus()
            mutableState.update {
                it.copy(
                    isLoading = false,
                    statusText = buildString {
                        append(status)
                        append('\n')
                        append(queryState.statusLine)
                    },
                    exercises = queryState.exercises,
                    personalRecords = queryState.prs,
                    expandedExerciseName = null,
                    expandedExerciseMarkdown = "",
                    expandedPrKey = null,
                    expandedPrMarkdown = "",
                )
            }
        }
    }

    override fun onQueryClick() {
        presenterScope.launch {
            mutableState.update {
                it.copy(
                    isLoading = true,
                    statusText = "Querying from sqlite...",
                )
            }

            val queryState = querySnapshotStatus()

            mutableState.update {
                it.copy(
                    isLoading = false,
                    statusText = queryState.statusLine,
                    exercises = queryState.exercises,
                    personalRecords = queryState.prs,
                    expandedExerciseName = null,
                    expandedExerciseMarkdown = "",
                    expandedPrKey = null,
                    expandedPrMarkdown = "",
                )
            }
        }
    }

    override fun onExerciseCardClick(exercise: ExerciseRecord) {
        val exerciseName = exercise.name
        mutableState.update { current ->
            if (current.expandedExerciseName == exerciseName) {
                current.copy(
                    expandedExerciseName = null,
                    expandedExerciseMarkdown = "",
                )
            } else {
                current.copy(
                    expandedExerciseName = exerciseName,
                    expandedExerciseMarkdown = buildExerciseMarkdown(exercise),
                )
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
                    expandedPrMarkdown = buildPrMarkdown(record),
                )
            }
        }
    }

    override fun dispose() {
        presenterScope.cancel()
    }

    private fun buildPrKey(record: PersonalRecord): String {
        return "${record.exerciseName}|${record.date}|${record.reps}|${record.maxWeight}"
    }

    private fun buildExerciseMarkdown(exercise: ExerciseRecord): String {
        return """
            # Exercise
            
            - Name: **${exercise.name}**
            - Type: `${exercise.type}`
            
            ## Query
            ```bash
            list --type ${exercise.type}
            ```
        """.trimIndent()
    }

    private fun buildPrMarkdown(record: PersonalRecord): String {
        val maxWeight = String.format(Locale.US, "%.1f", record.maxWeight)
        val oneRmEpley = String.format(Locale.US, "%.1f", record.estimatedOneRmEpley)
        val oneRmBrzycki = String.format(Locale.US, "%.1f", record.estimatedOneRmBrzycki)
        return """
            # Personal Record
            
            ## ${record.exerciseName}
            - Date: `${record.date}`
            - Max Weight: **${maxWeight} kg**
            - Reps: **${record.reps}**
            - 1RM (Epley): **${oneRmEpley} kg**
            - 1RM (Brzycki): **${oneRmBrzycki} kg**
        """.trimIndent()
    }

    private suspend fun querySnapshotStatus(): QueryState {
        val exercisesResult = repository.listExercises()
        val prsResult = repository.queryPrs()

        val exercises = if (exercisesResult.isSuccess()) {
            CorePayloadParser.parseExercises(exercisesResult.payloadJson)
        } else {
            emptyList()
        }
        val prs = if (prsResult.isSuccess()) {
            CorePayloadParser.parsePrs(prsResult.payloadJson)
        } else {
            emptyList()
        }

        val statusLine = buildString {
            append(exercisesResult.toStatusText(prefix = "List"))
            append('\n')
            append(prsResult.toStatusText(prefix = "PR"))
            if (exercisesResult.statusCode == CoreStatus.FILE_NOT_FOUND ||
                prsResult.statusCode == CoreStatus.FILE_NOT_FOUND
            ) {
                append('\n')
                append("Hint: ingest test data first.")
            }
        }
        return QueryState(
            statusLine = statusLine,
            exercises = exercises,
            prs = prs,
        )
    }

    private data class QueryState(
        val statusLine: String,
        val exercises: List<ExerciseRecord>,
        val prs: List<PersonalRecord>,
    )
}
