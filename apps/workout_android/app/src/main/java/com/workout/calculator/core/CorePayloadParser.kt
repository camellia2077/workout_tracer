package com.workout.calculator.core

import org.json.JSONObject

data class ExerciseRecord(
    val name: String,
    val type: String,
)

data class PersonalRecord(
    val exerciseName: String,
    val maxWeight: Double,
    val reps: Int,
    val date: String,
    val estimatedOneRmEpley: Double,
    val estimatedOneRmBrzycki: Double,
)

object CorePayloadParser {
    fun parseExercises(payloadJson: String): List<ExerciseRecord> {
        return runCatching {
            val root = JSONObject(payloadJson)
            val array = root.optJSONArray("exercises") ?: return emptyList()
            buildList(array.length()) {
                for (index in 0 until array.length()) {
                    val item = array.optJSONObject(index) ?: continue
                    add(
                        ExerciseRecord(
                            name = item.optString("name"),
                            type = item.optString("type"),
                        )
                    )
                }
            }
        }.getOrDefault(emptyList())
    }

    fun parsePrs(payloadJson: String): List<PersonalRecord> {
        return runCatching {
            val root = JSONObject(payloadJson)
            val array = root.optJSONArray("prs") ?: return emptyList()
            buildList(array.length()) {
                for (index in 0 until array.length()) {
                    val item = array.optJSONObject(index) ?: continue
                    add(
                        PersonalRecord(
                            exerciseName = item.optString("exercise_name"),
                            maxWeight = item.optDouble("max_weight"),
                            reps = item.optInt("reps"),
                            date = item.optString("date"),
                            estimatedOneRmEpley = item.optDouble("estimated_1rm_epley"),
                            estimatedOneRmBrzycki = item.optDouble("estimated_1rm_brzycki"),
                        )
                    )
                }
            }
        }.getOrDefault(emptyList())
    }
}
