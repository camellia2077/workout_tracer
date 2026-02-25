package com.workout.calculator.mvp

import android.content.Context
import com.workout.calculator.core.NativeResult
import com.workout.calculator.core.WorkoutNativeBridge
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

class NativeWorkoutRepository(
    private val appContext: Context,
) : WorkoutRepository {

    private val workingDir: File by lazy {
        File(appContext.filesDir, "mvp_core").also { it.mkdirs() }
    }

    override suspend fun ingestTestData(): NativeResult = withContext(Dispatchers.IO) {
        val input = copyAssetToWorkingDir(
            assetName = "2025_7.txt",
            targetName = "2025_7.txt",
        )
        val mapping = copyAssetToWorkingDir(
            assetName = "mapping.json",
            targetName = "mapping.json",
        )
        WorkoutNativeBridge.ingest(
            logPath = input.absolutePath,
            mappingPath = mapping.absolutePath,
            basePath = workingDir.absolutePath,
        )
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

    private fun copyAssetToWorkingDir(assetName: String, targetName: String): File {
        val target = File(workingDir, targetName)
        appContext.assets.open(assetName).use { input ->
            target.outputStream().use { output ->
                input.copyTo(output)
            }
        }
        return target
    }
}
