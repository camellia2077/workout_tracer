package com.workout.calculator.mvp

import com.workout.calculator.core.DisplayUnit
import com.workout.calculator.core.NativeResult

interface WorkoutRepository {
    suspend fun importFolder(folderUri: String): ImportFolderResult
    suspend fun exportArchive(targetUri: String): ExportArchiveResult
    suspend fun importArchive(archiveUri: String): ImportArchiveResult
    suspend fun clearDatabase(): NativeResult
    suspend fun clearTxtFiles(): NativeResult
    suspend fun listExercises(): NativeResult
    suspend fun queryPrs(): NativeResult
    suspend fun queryCycles(): NativeResult
    suspend fun queryCycleVolumes(cycleId: String): NativeResult
    suspend fun queryCycleTypeReport(
        cycleId: String,
        exerciseType: String,
        displayUnit: DisplayUnit,
    ): NativeResult
}
