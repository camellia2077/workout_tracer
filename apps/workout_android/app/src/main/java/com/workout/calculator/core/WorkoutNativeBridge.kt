package com.workout.calculator.core

object WorkoutNativeBridge {
    init {
        System.loadLibrary("workout_android")
    }

    external fun ingest(logPath: String, mappingPath: String, basePath: String): NativeResult

    external fun rebuildFromLogs(
        logsPath: String,
        mappingPath: String,
        basePath: String,
    ): NativeResult

    external fun exportArchive(basePath: String, archiveOutputPath: String): NativeResult

    external fun importArchive(basePath: String, archivePath: String): NativeResult

    external fun listExercises(basePath: String, typeFilter: String): NativeResult

    external fun queryPrs(basePath: String): NativeResult

    external fun queryCycles(basePath: String): NativeResult

    external fun queryCycleVolumes(basePath: String, cycleId: String): NativeResult

    external fun queryCycleTypeReport(
        basePath: String,
        cycleId: String,
        exerciseType: String,
        displayUnit: String,
    ): NativeResult

    external fun getCoreVersion(): String
}
