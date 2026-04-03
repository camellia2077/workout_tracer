package com.workout.calculator.core

import org.junit.Assert.assertEquals
import org.junit.Test

class DisplayUnitFormatterTest {

    @Test
    fun formatDetailWeight_usesOriginalValueForOriginalMode() {
        val text = DisplayUnitFormatter.formatDetailWeight(
            weightKg = -29.93709642,
            originalUnit = "lb",
            originalWeightValue = -66.0,
            displayUnit = DisplayUnit.Original,
        )

        assertEquals("-66 lb", text)
    }

    @Test
    fun formatDetailWeight_convertsForKgAndLbModes() {
        val weightKg = 45.359237

        val kgText = DisplayUnitFormatter.formatDetailWeight(
            weightKg = weightKg,
            originalUnit = "lb",
            originalWeightValue = 100.0,
            displayUnit = DisplayUnit.Kg,
        )
        val lbText = DisplayUnitFormatter.formatDetailWeight(
            weightKg = weightKg,
            originalUnit = "kg",
            originalWeightValue = 45.359237,
            displayUnit = DisplayUnit.Lb,
        )

        assertEquals("45.359 kg", kgText)
        assertEquals("100 lb", lbText)
    }

    @Test
    fun formatAggregateMetric_usesCommonOriginalUnitWhenAvailable() {
        val text = DisplayUnitFormatter.formatAggregateMetric(
            valueKg = 45.359237,
            commonOriginalUnit = "lb",
            displayUnit = DisplayUnit.Original,
        )

        assertEquals("100.0 lb", text)
    }

    @Test
    fun formatAggregateMetric_fallsBackToKgForMixedOriginalUnits() {
        val text = DisplayUnitFormatter.formatAggregateMetric(
            valueKg = 100.0,
            commonOriginalUnit = "",
            displayUnit = DisplayUnit.Original,
        )

        assertEquals("100.0 kg", text)
    }

    @Test
    fun formatOneRmOrDash_returnsDashForNonPositiveReferenceWeight() {
        val text = DisplayUnitFormatter.formatOneRmOrDash(
            oneRmKg = 60.0,
            referenceWeightKg = 0.0,
            originalUnit = "lb",
            displayUnit = DisplayUnit.Original,
        )

        assertEquals("-", text)
    }
}
