package com.workout.calculator.core

object WorkoutNativeBridge {
    init {
        System.loadLibrary("workout_android")
    }

    external fun ingest(logPath: String, mappingPath: String, basePath: String): NativeResult

    external fun listExercises(basePath: String, typeFilter: String): NativeResult

    external fun queryPrs(basePath: String): NativeResult

    external fun getCoreVersion(): String
}
