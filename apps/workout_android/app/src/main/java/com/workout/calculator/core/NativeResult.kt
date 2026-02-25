package com.workout.calculator.core

data class NativeResult(
    val statusCode: Int,
    val message: String,
    val payloadJson: String,
)
