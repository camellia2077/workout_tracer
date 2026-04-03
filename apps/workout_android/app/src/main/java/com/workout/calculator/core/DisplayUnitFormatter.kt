package com.workout.calculator.core

enum class DisplayUnit {
    Original,
    Kg,
    Lb,
}

data class DisplayAmount(
    val value: Double,
    val unit: String,
)

object DisplayUnitFormatter {
    private const val LB_TO_KG = 0.45359237
    private const val KG_UNIT = "kg"
    private const val LB_UNIT = "lb"

    fun resolveDetailAmount(
        weightKg: Double,
        originalUnit: String,
        originalWeightValue: Double,
        displayUnit: DisplayUnit,
    ): DisplayAmount {
        val unit = when (displayUnit) {
            DisplayUnit.Original -> normalizeOriginalUnit(originalUnit) ?: KG_UNIT
            DisplayUnit.Kg -> KG_UNIT
            DisplayUnit.Lb -> LB_UNIT
        }
        val value = when (displayUnit) {
            DisplayUnit.Original -> originalWeightValue
            DisplayUnit.Kg -> weightKg
            DisplayUnit.Lb -> convertFromKg(weightKg, LB_UNIT)
        }
        return DisplayAmount(value = value, unit = unit)
    }

    fun resolveAggregateAmount(
        valueKg: Double,
        commonOriginalUnit: String,
        displayUnit: DisplayUnit,
    ): DisplayAmount {
        val unit = resolveAggregateUnit(displayUnit, commonOriginalUnit)
        return DisplayAmount(value = convertFromKg(valueKg, unit), unit = unit)
    }

    fun resolveAggregateUnit(
        displayUnit: DisplayUnit,
        commonOriginalUnit: String,
    ): String {
        return when (displayUnit) {
            DisplayUnit.Original -> normalizeOriginalUnit(commonOriginalUnit) ?: KG_UNIT
            DisplayUnit.Kg -> KG_UNIT
            DisplayUnit.Lb -> LB_UNIT
        }
    }

    fun formatWeight(value: Double, precision: Int = 3): String {
        val normalized = if (kotlin.math.abs(value) < 0.0000005) 0.0 else value
        val raw = "%.${precision}f".format(java.util.Locale.US, normalized)
        return raw.trimEnd('0').trimEnd('.').ifEmpty { "0" }
    }

    fun formatMetric(value: Double, precision: Int = 1): String {
        return "%.${precision}f".format(java.util.Locale.US, value)
    }

    fun formatDetailWeight(
        weightKg: Double,
        originalUnit: String,
        originalWeightValue: Double,
        displayUnit: DisplayUnit,
    ): String {
        val amount = resolveDetailAmount(
            weightKg = weightKg,
            originalUnit = originalUnit,
            originalWeightValue = originalWeightValue,
            displayUnit = displayUnit,
        )
        return "${formatWeight(amount.value)} ${amount.unit}"
    }

    fun formatAggregateMetric(
        valueKg: Double,
        commonOriginalUnit: String,
        displayUnit: DisplayUnit,
    ): String {
        val amount = resolveAggregateAmount(
            valueKg = valueKg,
            commonOriginalUnit = commonOriginalUnit,
            displayUnit = displayUnit,
        )
        return "${formatMetric(amount.value)} ${amount.unit}"
    }

    fun formatOneRmOrDash(
        oneRmKg: Double,
        referenceWeightKg: Double,
        originalUnit: String,
        displayUnit: DisplayUnit,
    ): String {
        if (referenceWeightKg <= 0.0) {
            return "-"
        }
        val amount = when (displayUnit) {
            DisplayUnit.Original ->
                DisplayAmount(
                    value = convertFromKg(oneRmKg, normalizeOriginalUnit(originalUnit) ?: KG_UNIT),
                    unit = normalizeOriginalUnit(originalUnit) ?: KG_UNIT,
                )

            DisplayUnit.Kg -> DisplayAmount(oneRmKg, KG_UNIT)
            DisplayUnit.Lb -> DisplayAmount(convertFromKg(oneRmKg, LB_UNIT), LB_UNIT)
        }
        return "${formatMetric(amount.value)} ${amount.unit}"
    }

    fun formatPercent(partKg: Double, totalKg: Double): String {
        val percent = if (totalKg <= 0.0) 0.0 else (partKg / totalKg) * 100.0
        return "${formatMetric(percent)}%"
    }

    private fun convertFromKg(valueKg: Double, unit: String): Double {
        return if (unit == LB_UNIT) valueKg / LB_TO_KG else valueKg
    }

    private fun normalizeOriginalUnit(unit: String): String? {
        return when (unit.lowercase()) {
            "", KG_UNIT -> KG_UNIT
            "l", "lb", "lbs" -> LB_UNIT
            else -> null
        }
    }
}
