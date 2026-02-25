package com.workout.calculator.core

object CoreStatus {
    const val SUCCESS = 0
    const val INVALID_ARGS = 1
    const val VALIDATION_ERROR = 2
    const val FILE_NOT_FOUND = 3
    const val DATABASE_ERROR = 4
    const val PROCESSING_ERROR = 5
    const val UNKNOWN_ERROR = 99
}

fun NativeResult.isSuccess(): Boolean = statusCode == CoreStatus.SUCCESS

fun NativeResult.toStatusText(prefix: String): String {
    val normalizedMessage = message.ifBlank { "ok" }
    return "$prefix: code=$statusCode, message=$normalizedMessage"
}
