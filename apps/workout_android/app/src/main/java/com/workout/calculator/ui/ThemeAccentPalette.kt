package com.workout.calculator.ui

import androidx.compose.ui.graphics.Color
import com.workout.calculator.mvp.AccentColor

data class AccentPalette(
    val lightPrimary: Color,
    val lightPrimaryContainer: Color,
    val darkPrimary: Color,
    val darkPrimaryContainer: Color,
)

fun accentColorLabel(color: AccentColor): String {
    return when (color) {
        AccentColor.Red -> "Red"
        AccentColor.Orange -> "Orange"
        AccentColor.Yellow -> "Yellow"
        AccentColor.Green -> "Green"
        AccentColor.Cyan -> "Cyan"
        AccentColor.Blue -> "Blue"
        AccentColor.Purple -> "Purple"
    }
}

fun accentPaletteFor(color: AccentColor): AccentPalette {
    return when (color) {
        AccentColor.Red -> AccentPalette(
            lightPrimary = Color(0xFFB3261E),
            lightPrimaryContainer = Color(0xFFFFDAD6),
            darkPrimary = Color(0xFFF2B8B5),
            darkPrimaryContainer = Color(0xFF8C1D18),
        )

        AccentColor.Orange -> AccentPalette(
            lightPrimary = Color(0xFF9A4600),
            lightPrimaryContainer = Color(0xFFFFDBCA),
            darkPrimary = Color(0xFFFFB68A),
            darkPrimaryContainer = Color(0xFF7A3A00),
        )

        AccentColor.Yellow -> AccentPalette(
            lightPrimary = Color(0xFF7A6000),
            lightPrimaryContainer = Color(0xFFFCE5A0),
            darkPrimary = Color(0xFFE8C36A),
            darkPrimaryContainer = Color(0xFF5D4700),
        )

        AccentColor.Green -> AccentPalette(
            lightPrimary = Color(0xFF386A20),
            lightPrimaryContainer = Color(0xFFB7F397),
            darkPrimary = Color(0xFFA2D485),
            darkPrimaryContainer = Color(0xFF23510A),
        )

        AccentColor.Cyan -> AccentPalette(
            lightPrimary = Color(0xFF006A6A),
            lightPrimaryContainer = Color(0xFF9FF1F0),
            darkPrimary = Color(0xFF7ADBD9),
            darkPrimaryContainer = Color(0xFF004F4F),
        )

        AccentColor.Blue -> AccentPalette(
            lightPrimary = Color(0xFF0B57D0),
            lightPrimaryContainer = Color(0xFFD6E3FF),
            darkPrimary = Color(0xFFADC6FF),
            darkPrimaryContainer = Color(0xFF0040A8),
        )

        AccentColor.Purple -> AccentPalette(
            lightPrimary = Color(0xFF6750A4),
            lightPrimaryContainer = Color(0xFFEADDFF),
            darkPrimary = Color(0xFFD0BCFF),
            darkPrimaryContainer = Color(0xFF4F378B),
        )
    }
}
