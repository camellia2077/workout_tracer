package com.workout.calculator.mvp

import com.workout.calculator.core.CoreStatus
import com.workout.calculator.core.DisplayUnit
import com.workout.calculator.core.NativeResult
import com.workout.calculator.testutil.MainDispatcherRule
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class WorkoutPresenterTest {

    @get:Rule
    val mainDispatcherRule = MainDispatcherRule()

    @Test
    fun init_loadsSavedDisplayUnit() = runTest {
        val preferences = FakeUiPreferenceStorage(displayUnit = DisplayUnit.Lb)

        val presenter = WorkoutPresenter(
            repository = FakeWorkoutRepository(),
            preferenceStorage = preferences,
        )

        assertEquals(DisplayUnit.Lb, presenter.state.value.displayUnit)
        presenter.dispose()
    }

    @Test
    fun onQueryClick_buildsMonthOptionsAndAutoLoadsSquatReport() = runTest {
        val repository = FakeWorkoutRepository(
            prsResult = successPayload("""{"prs":[]}"""),
            cyclesResult = successPayload(
                """
                    {
                      "cycles": [
                        {
                          "cycle_id": "2025-06",
                          "total_days": 2,
                          "type": "push,pull,squat",
                          "start_date": "2025-06-02",
                          "end_date": "2025-06-04"
                        },
                        {
                          "cycle_id": "2025-07",
                          "total_days": 2,
                          "type": "push,pull,squat",
                          "start_date": "2025-07-02",
                          "end_date": "2025-07-04"
                        }
                      ]
                    }
                """.trimIndent()
            ),
            reportResults = mutableMapOf(
                "2025-07|squat|original" to successPayload(
                    """
                        {
                          "cycle_id": "2025-07",
                          "exercise_type": "squat",
                          "markdown": "# Squat 2025-07"
                        }
                    """.trimIndent()
                )
            ),
        )
        val presenter = WorkoutPresenter(repository, FakeUiPreferenceStorage())

        presenter.onQueryClick()

        val state = presenter.state.value
        assertEquals(listOf("2025-07", "2025-06"), state.monthOptions)
        assertEquals("2025-07", state.selectedMonth)
        assertEquals(QueryType.Squat, state.selectedType)
        assertEquals("# Squat 2025-07", state.currentReportMarkdown)
        assertEquals(1, repository.queryCycleTypeReportCalls["2025-07|squat|original"] ?: 0)
        presenter.dispose()
    }

    @Test
    fun onMonthSelected_resetsToSquatAndFetchesSelectedMonthReport() = runTest {
        val repository = FakeWorkoutRepository(
            cyclesResult = successPayload(
                """
                    {
                      "cycles": [
                        {
                          "cycle_id": "2025-07",
                          "total_days": 2,
                          "type": "push,pull,squat",
                          "start_date": "2025-07-02",
                          "end_date": "2025-07-04"
                        },
                        {
                          "cycle_id": "2025-06",
                          "total_days": 2,
                          "type": "push,pull,squat",
                          "start_date": "2025-06-02",
                          "end_date": "2025-06-04"
                        }
                      ]
                    }
                """.trimIndent()
            ),
            reportResults = mutableMapOf(
                "2025-07|squat|original" to successPayload(
                    """{"cycle_id":"2025-07","exercise_type":"squat","markdown":"# Squat 2025-07"}"""
                ),
                "2025-06|squat|original" to successPayload(
                    """{"cycle_id":"2025-06","exercise_type":"squat","markdown":"# Squat 2025-06"}"""
                ),
            ),
        )
        val presenter = WorkoutPresenter(repository, FakeUiPreferenceStorage())

        presenter.onQueryClick()
        presenter.onMonthSelected("2025-06")

        val state = presenter.state.value
        assertEquals("2025-06", state.selectedMonth)
        assertEquals(QueryType.Squat, state.selectedType)
        assertEquals("# Squat 2025-06", state.currentReportMarkdown)
        assertEquals(1, repository.queryCycleTypeReportCalls["2025-06|squat|original"] ?: 0)
        presenter.dispose()
    }

    @Test
    fun onTypeSelected_requestsEveryClickWithoutCaching() = runTest {
        val repository = FakeWorkoutRepository(
            cyclesResult = successPayload(
                """
                    {
                      "cycles": [
                        {
                          "cycle_id": "2025-07",
                          "total_days": 2,
                          "type": "push,pull,squat",
                          "start_date": "2025-07-02",
                          "end_date": "2025-07-04"
                        }
                      ]
                    }
                """.trimIndent()
            ),
            reportResults = mutableMapOf(
                "2025-07|squat|original" to successPayload(
                    """{"cycle_id":"2025-07","exercise_type":"squat","markdown":"# Squat"}"""
                ),
                "2025-07|pull|original" to successPayload(
                    """{"cycle_id":"2025-07","exercise_type":"pull","markdown":"# Pull"}"""
                ),
            ),
        )
        val presenter = WorkoutPresenter(repository, FakeUiPreferenceStorage())

        presenter.onQueryClick()
        presenter.onTypeSelected(QueryType.Pull)
        presenter.onTypeSelected(QueryType.Pull)

        assertEquals(QueryType.Pull, presenter.state.value.selectedType)
        assertEquals("# Pull", presenter.state.value.currentReportMarkdown)
        assertEquals(2, repository.queryCycleTypeReportCalls["2025-07|pull|original"] ?: 0)
        presenter.dispose()
    }

    @Test
    fun onDisplayUnitSelected_refetchesCurrentMonthAndTypeReport() = runTest {
        val repository = FakeWorkoutRepository(
            cyclesResult = successPayload(
                """
                    {
                      "cycles": [
                        {
                          "cycle_id": "2025-07",
                          "total_days": 2,
                          "type": "push,pull,squat",
                          "start_date": "2025-07-02",
                          "end_date": "2025-07-04"
                        }
                      ]
                    }
                """.trimIndent()
            ),
            reportResults = mutableMapOf(
                "2025-07|squat|original" to successPayload(
                    """{"cycle_id":"2025-07","exercise_type":"squat","markdown":"# Squat Original"}"""
                ),
                "2025-07|push|original" to successPayload(
                    """{"cycle_id":"2025-07","exercise_type":"push","markdown":"# Push Original"}"""
                ),
                "2025-07|push|kg" to successPayload(
                    """{"cycle_id":"2025-07","exercise_type":"push","markdown":"# Push KG"}"""
                ),
            ),
        )
        val preferences = FakeUiPreferenceStorage(displayUnit = DisplayUnit.Original)
        val presenter = WorkoutPresenter(repository, preferences)

        presenter.onQueryClick()
        presenter.onTypeSelected(QueryType.Push)
        presenter.onDisplayUnitSelected(DisplayUnit.Kg)

        assertEquals(DisplayUnit.Kg, presenter.state.value.displayUnit)
        assertEquals("# Push KG", presenter.state.value.currentReportMarkdown)
        assertEquals(1, repository.queryCycleTypeReportCalls["2025-07|push|original"] ?: 0)
        assertEquals(1, repository.queryCycleTypeReportCalls["2025-07|push|kg"] ?: 0)
        assertEquals(DisplayUnit.Kg, preferences.savedDisplayUnit)
        presenter.dispose()
    }

    @Test
    fun onQueryClick_withEmptyCycles_keepsMonthAndReportEmpty() = runTest {
        val repository = FakeWorkoutRepository(
            prsResult = successPayload("""{"prs":[]}"""),
            cyclesResult = successPayload("""{"cycles":[]}"""),
        )
        val presenter = WorkoutPresenter(repository, FakeUiPreferenceStorage())

        presenter.onQueryClick()

        val state = presenter.state.value
        assertTrue(state.monthOptions.isEmpty())
        assertNull(state.selectedMonth)
        assertTrue(state.currentReportMarkdown.isEmpty())
        assertFalse(state.isReportLoading)
        presenter.dispose()
    }

    @Test
    fun onImportFolderSelected_refreshesMonthsAndAutoLoadsSquatWhenImportSucceeds() = runTest {
        val repository = FakeWorkoutRepository(
            importResult = ImportFolderResult(
                copiedTxtCount = 1,
                processedTxtCount = 1,
                importedTxtCount = 1,
                failedFiles = emptyList(),
                statusCode = CoreStatus.SUCCESS,
                message = "ok",
            ),
            cyclesResult = successPayload(
                """
                    {
                      "cycles": [
                        {
                          "cycle_id": "2026-01",
                          "total_days": 2,
                          "type": "push,pull,squat",
                          "start_date": "2026-01-02",
                          "end_date": "2026-01-05"
                        }
                      ]
                    }
                """.trimIndent()
            ),
            reportResults = mutableMapOf(
                "2026-01|squat|original" to successPayload(
                    """{"cycle_id":"2026-01","exercise_type":"squat","markdown":"# Squat 2026-01"}"""
                )
            ),
        )
        val presenter = WorkoutPresenter(repository, FakeUiPreferenceStorage())

        presenter.onImportFolderSelected("content://picked/folder")

        assertEquals(1, repository.importFolderCalls)
        assertEquals(listOf("2026-01"), presenter.state.value.monthOptions)
        assertEquals("# Squat 2026-01", presenter.state.value.currentReportMarkdown)
        presenter.dispose()
    }

    private class FakeWorkoutRepository(
        private val importResult: ImportFolderResult = ImportFolderResult(
            copiedTxtCount = 0,
            processedTxtCount = 0,
            importedTxtCount = 0,
            failedFiles = emptyList(),
            statusCode = CoreStatus.SUCCESS,
            message = "",
        ),
        private val exportArchiveResult: ExportArchiveResult = ExportArchiveResult(
            recordsCount = 0,
            jsonCount = 0,
            statusCode = CoreStatus.SUCCESS,
            message = "",
        ),
        private val importArchiveResult: ImportArchiveResult = ImportArchiveResult(
            processedTxtCount = 0,
            importedTxtCount = 0,
            failedFiles = emptyList(),
            statusCode = CoreStatus.SUCCESS,
            message = "",
        ),
        private val exercisesResult: NativeResult = successPayload("""{"exercises":[]}"""),
        private val prsResult: NativeResult = successPayload("""{"prs":[]}"""),
        private val cyclesResult: NativeResult = successPayload("""{"cycles":[]}"""),
        private val reportResults: MutableMap<String, NativeResult> = mutableMapOf(),
    ) : WorkoutRepository {

        var importFolderCalls: Int = 0
            private set

        var exportArchiveCalls: Int = 0
            private set

        var importArchiveCalls: Int = 0
            private set

        val queryCycleTypeReportCalls: MutableMap<String, Int> = mutableMapOf()

        override suspend fun importFolder(folderUri: String): ImportFolderResult {
            importFolderCalls += 1
            return importResult
        }

        override suspend fun exportArchive(targetUri: String): ExportArchiveResult {
            exportArchiveCalls += 1
            return exportArchiveResult
        }

        override suspend fun importArchive(archiveUri: String): ImportArchiveResult {
            importArchiveCalls += 1
            return importArchiveResult
        }

        override suspend fun clearDatabase(): NativeResult {
            return successPayload("{}")
        }

        override suspend fun clearTxtFiles(): NativeResult {
            return successPayload("{}")
        }

        override suspend fun listExercises(): NativeResult = exercisesResult

        override suspend fun queryPrs(): NativeResult = prsResult

        override suspend fun queryCycles(): NativeResult = cyclesResult

        override suspend fun queryCycleVolumes(cycleId: String): NativeResult {
            return successPayload("""{"volumes":[]}""")
        }

        override suspend fun queryCycleTypeReport(
            cycleId: String,
            exerciseType: String,
            displayUnit: DisplayUnit,
        ): NativeResult {
            val key = "$cycleId|$exerciseType|${displayUnit.toCoreValue()}"
            queryCycleTypeReportCalls[key] = (queryCycleTypeReportCalls[key] ?: 0) + 1
            return reportResults[key] ?: NativeResult(
                statusCode = CoreStatus.PROCESSING_ERROR,
                message = "report markdown not found",
                payloadJson = "{}",
            )
        }

        private fun DisplayUnit.toCoreValue(): String {
            return when (this) {
                DisplayUnit.Original -> "original"
                DisplayUnit.Kg -> "kg"
                DisplayUnit.Lb -> "lb"
            }
        }
    }

    private class FakeUiPreferenceStorage(
        private var themeMode: ThemeMode = ThemeMode.Light,
        private var accentColor: AccentColor = AccentColor.Blue,
        private var displayUnit: DisplayUnit = DisplayUnit.Original,
    ) : UiPreferenceStorage {

        var savedDisplayUnit: DisplayUnit? = null
            private set

        override suspend fun loadThemeMode(): ThemeMode = themeMode

        override suspend fun saveThemeMode(mode: ThemeMode) {
            themeMode = mode
        }

        override suspend fun loadAccentColor(): AccentColor = accentColor

        override suspend fun saveAccentColor(color: AccentColor) {
            accentColor = color
        }

        override suspend fun loadDisplayUnit(): DisplayUnit = displayUnit

        override suspend fun saveDisplayUnit(displayUnit: DisplayUnit) {
            savedDisplayUnit = displayUnit
            this.displayUnit = displayUnit
        }
    }
}

private fun successPayload(payloadJson: String): NativeResult {
    return NativeResult(
        statusCode = CoreStatus.SUCCESS,
        message = "",
        payloadJson = payloadJson,
    )
}
