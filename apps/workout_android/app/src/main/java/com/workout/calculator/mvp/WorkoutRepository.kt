package com.workout.calculator.mvp

import com.workout.calculator.core.NativeResult

interface WorkoutRepository {
    suspend fun ingestTestData(): NativeResult
    suspend fun listExercises(): NativeResult
    suspend fun queryPrs(): NativeResult
}
