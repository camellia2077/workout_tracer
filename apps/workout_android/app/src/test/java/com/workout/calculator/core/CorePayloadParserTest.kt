package com.workout.calculator.core

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class CorePayloadParserTest {

    @Test
    fun parsePrs_readsNormalizedWeightFields() {
        val payload = """
            {
              "prs": [
                {
                  "exercise_name": "Pull Up",
                  "max_weight_kg": -29.93709642,
                  "original_unit": "lb",
                  "original_weight_value": -66,
                  "reps": 3,
                  "date": "2025-07-05",
                  "estimated_1rm_epley_kg": 0,
                  "estimated_1rm_brzycki_kg": 0
                }
              ]
            }
        """.trimIndent()

        val records = CorePayloadParser.parsePrs(payload)

        assertEquals(1, records.size)
        assertEquals("Pull Up", records.single().exerciseName)
        assertEquals(-29.93709642, records.single().maxWeightKg, 0.0000001)
        assertEquals("lb", records.single().originalUnit)
        assertEquals(-66.0, records.single().originalWeightValue, 0.0)
        assertEquals(3, records.single().reps)
        assertEquals("2025-07-05", records.single().date)
    }

    @Test
    fun parseCycles_readsCyclePayload() {
        val payload = """
            {
              "cycles": [
                {
                  "cycle_id": "2025-07",
                  "total_days": 1,
                  "type": "pull,push,squat",
                  "start_date": "2025-07-05",
                  "end_date": "2025-07-05"
                }
              ]
            }
        """.trimIndent()

        val records = CorePayloadParser.parseCycles(payload)

        assertEquals(1, records.size)
        assertEquals("2025-07", records.single().cycleId)
        assertEquals(1, records.single().totalDays)
        assertEquals("pull,push,squat", records.single().type)
        assertEquals("2025-07-05", records.single().startDate)
        assertEquals("2025-07-05", records.single().endDate)
    }

    @Test
    fun parseCycleVolumes_readsAggregatePayload() {
        val payload = """
            {
              "volumes": [
                {
                  "cycle_id": "2025-07",
                  "exercise_type": "pull",
                  "total_volume_kg": 120.5,
                  "common_original_unit": "",
                  "total_days": 1,
                  "average_intensity_kg": 20.0833,
                  "session_count": 1,
                  "total_reps": 6,
                  "total_sets": 2,
                  "vol_power_kg": 120.5,
                  "vol_hypertrophy_kg": 0,
                  "vol_endurance_kg": 0
                }
              ]
            }
        """.trimIndent()

        val records = CorePayloadParser.parseCycleVolumes(payload)

        assertEquals(1, records.size)
        assertEquals("2025-07", records.single().cycleId)
        assertEquals("pull", records.single().exerciseType)
        assertEquals(120.5, records.single().totalVolumeKg, 0.0)
        assertTrue(records.single().commonOriginalUnit.isEmpty())
        assertEquals(1, records.single().sessionCount)
        assertEquals(6, records.single().totalReps)
        assertEquals(2, records.single().totalSets)
    }

    @Test
    fun parseImportSummary_readsRebuildPayload() {
        val payload = """
            {
              "processed_txt_count": 3,
              "imported_txt_count": 2,
              "failed_txt_count": 1,
              "failed_files": [
                "nested/invalid.txt"
              ],
              "failed_details": [
                {
                  "file": "nested/invalid.txt",
                  "stage": "process_file",
                  "diagnostics": [
                    "Error: [Validator] Invalid format at line 7."
                  ]
                }
              ]
            }
        """.trimIndent()

        val summary = CorePayloadParser.parseImportSummary(payload)

        assertEquals(3, summary.processedTxtCount)
        assertEquals(2, summary.importedTxtCount)
        assertEquals(1, summary.failedTxtCount)
        assertEquals(listOf("nested/invalid.txt"), summary.failedFiles)
        assertEquals(1, summary.failedDetails.size)
        assertEquals("nested/invalid.txt", summary.failedDetails.single().file)
        assertEquals("process_file", summary.failedDetails.single().stage)
        assertEquals(
            listOf("Error: [Validator] Invalid format at line 7."),
            summary.failedDetails.single().diagnostics,
        )
    }

    @Test
    fun parseArchiveExportSummary_readsArchivePayload() {
        val payload = """
            {
              "records_count": 2,
              "json_count": 2,
              "archive_output_path": "/tmp/workout_backup.zip"
            }
        """.trimIndent()

        val summary = CorePayloadParser.parseArchiveExportSummary(payload)

        assertEquals(2, summary.recordsCount)
        assertEquals(2, summary.jsonCount)
        assertEquals("/tmp/workout_backup.zip", summary.archiveOutputPath)
    }

    @Test
    fun parseCycleTypeReport_readsMarkdownPayload() {
        val payload = """
            {
              "cycle_id": "2025-07",
              "exercise_type": "squat",
              "markdown": "# Squat Training Report"
            }
        """.trimIndent()

        val report = CorePayloadParser.parseCycleTypeReport(payload)

        requireNotNull(report)
        assertEquals("2025-07", report.cycleId)
        assertEquals("squat", report.exerciseType)
        assertEquals("# Squat Training Report", report.markdown)
    }
}
