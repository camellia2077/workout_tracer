package com.workout.calculator.core

import org.json.JSONObject

data class ExerciseRecord(
    val name: String,
    val type: String,
)

data class PersonalRecord(
    val exerciseName: String,
    val maxWeightKg: Double,
    val originalUnit: String,
    val originalWeightValue: Double,
    val reps: Int,
    val date: String,
    val estimatedOneRmEpleyKg: Double,
    val estimatedOneRmBrzyckiKg: Double,
)

data class CycleRecord(
    val cycleId: String,
    val totalDays: Int,
    val type: String,
    val startDate: String,
    val endDate: String,
)

data class VolumeStatsRecord(
    val cycleId: String,
    val exerciseType: String,
    val totalVolumeKg: Double,
    val commonOriginalUnit: String,
    val totalDays: Int,
    val averageIntensityKg: Double,
    val sessionCount: Int,
    val totalReps: Int,
    val totalSets: Int,
    val volPowerKg: Double,
    val volHypertrophyKg: Double,
    val volEnduranceKg: Double,
)

data class ReportMarkdownRecord(
    val cycleId: String,
    val exerciseType: String,
    val markdown: String,
)

data class ImportSummaryRecord(
    val processedTxtCount: Int,
    val importedTxtCount: Int,
    val failedTxtCount: Int,
    val failedFiles: List<String>,
    val failedDetails: List<ImportFailureDetailRecord>,
)

data class ImportFailureDetailRecord(
    val file: String,
    val stage: String,
    val diagnostics: List<String>,
)

data class ArchiveExportSummaryRecord(
    val recordsCount: Int,
    val jsonCount: Int,
    val archiveOutputPath: String,
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
                            maxWeightKg = item.optDouble("max_weight_kg"),
                            originalUnit = item.optString("original_unit", "kg"),
                            originalWeightValue = item.optDouble("original_weight_value"),
                            reps = item.optInt("reps"),
                            date = item.optString("date"),
                            estimatedOneRmEpleyKg =
                                item.optDouble("estimated_1rm_epley_kg"),
                            estimatedOneRmBrzyckiKg =
                                item.optDouble("estimated_1rm_brzycki_kg"),
                        )
                    )
                }
            }
        }.getOrDefault(emptyList())
    }

    fun parseCycles(payloadJson: String): List<CycleRecord> {
        return runCatching {
            val root = JSONObject(payloadJson)
            val array = root.optJSONArray("cycles") ?: return emptyList()
            buildList(array.length()) {
                for (index in 0 until array.length()) {
                    val item = array.optJSONObject(index) ?: continue
                    add(
                        CycleRecord(
                            cycleId = item.optString("cycle_id"),
                            totalDays = item.optInt("total_days"),
                            type = item.optString("type"),
                            startDate = item.optString("start_date"),
                            endDate = item.optString("end_date"),
                        )
                    )
                }
            }
        }.getOrDefault(emptyList())
    }

    fun parseCycleVolumes(payloadJson: String): List<VolumeStatsRecord> {
        return runCatching {
            val root = JSONObject(payloadJson)
            val array = root.optJSONArray("volumes") ?: return emptyList()
            buildList(array.length()) {
                for (index in 0 until array.length()) {
                    val item = array.optJSONObject(index) ?: continue
                    add(
                        VolumeStatsRecord(
                            cycleId = item.optString("cycle_id"),
                            exerciseType = item.optString("exercise_type"),
                            totalVolumeKg = item.optDouble("total_volume_kg"),
                            commonOriginalUnit =
                                item.optString("common_original_unit"),
                            totalDays = item.optInt("total_days"),
                            averageIntensityKg =
                                item.optDouble("average_intensity_kg"),
                            sessionCount = item.optInt("session_count"),
                            totalReps = item.optInt("total_reps"),
                            totalSets = item.optInt("total_sets"),
                            volPowerKg = item.optDouble("vol_power_kg"),
                            volHypertrophyKg =
                                item.optDouble("vol_hypertrophy_kg"),
                            volEnduranceKg = item.optDouble("vol_endurance_kg"),
                        )
                    )
                }
            }
        }.getOrDefault(emptyList())
    }

    fun parseCycleTypeReport(payloadJson: String): ReportMarkdownRecord? {
        return runCatching {
            val root = JSONObject(payloadJson)
            ReportMarkdownRecord(
                cycleId = root.optString("cycle_id"),
                exerciseType = root.optString("exercise_type"),
                markdown = root.optString("markdown"),
            )
        }.getOrNull()?.takeIf {
            it.cycleId.isNotBlank() && it.exerciseType.isNotBlank() && it.markdown.isNotBlank()
        }
    }

    fun parseImportSummary(payloadJson: String): ImportSummaryRecord {
        return runCatching {
            val root = JSONObject(payloadJson)
            val failedArray = root.optJSONArray("failed_files")
            val failedFiles = buildList(failedArray?.length() ?: 0) {
                if (failedArray == null) {
                    return@buildList
                }
                for (index in 0 until failedArray.length()) {
                    val item = failedArray.optString(index)
                    if (item.isNotBlank()) {
                        add(item)
                    }
                }
            }
            val detailArray = root.optJSONArray("failed_details")
            val failedDetails = buildList(detailArray?.length() ?: 0) {
                if (detailArray == null) {
                    return@buildList
                }
                for (index in 0 until detailArray.length()) {
                    val item = detailArray.optJSONObject(index) ?: continue
                    val diagnosticsArray = item.optJSONArray("diagnostics")
                    val diagnostics = buildList(diagnosticsArray?.length() ?: 0) {
                        if (diagnosticsArray == null) {
                            return@buildList
                        }
                        for (diagIndex in 0 until diagnosticsArray.length()) {
                            val diagnostic = diagnosticsArray.optString(diagIndex)
                            if (diagnostic.isNotBlank()) {
                                add(diagnostic)
                            }
                        }
                    }
                    add(
                        ImportFailureDetailRecord(
                            file = item.optString("file"),
                            stage = item.optString("stage"),
                            diagnostics = diagnostics,
                        )
                    )
                }
            }
            ImportSummaryRecord(
                processedTxtCount = root.optInt("processed_txt_count"),
                importedTxtCount = root.optInt("imported_txt_count"),
                failedTxtCount = root.optInt("failed_txt_count", failedFiles.size),
                failedFiles = failedFiles,
                failedDetails = failedDetails,
            )
        }.getOrDefault(
            ImportSummaryRecord(
                processedTxtCount = 0,
                importedTxtCount = 0,
                failedTxtCount = 0,
                failedFiles = emptyList(),
                failedDetails = emptyList(),
            )
        )
    }

    fun parseArchiveExportSummary(payloadJson: String): ArchiveExportSummaryRecord {
        return runCatching {
            val root = JSONObject(payloadJson)
            ArchiveExportSummaryRecord(
                recordsCount = root.optInt("records_count"),
                jsonCount = root.optInt("json_count"),
                archiveOutputPath = root.optString("archive_output_path"),
            )
        }.getOrDefault(
            ArchiveExportSummaryRecord(
                recordsCount = 0,
                jsonCount = 0,
                archiveOutputPath = "",
            )
        )
    }
}
