package com.workout.calculator.mvp

import android.content.Context
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.map

private val Context.themeDataStore by preferencesDataStore(name = "workout_user_prefs")

interface UiPreferenceStorage {
    suspend fun loadThemeMode(): ThemeMode
    suspend fun saveThemeMode(mode: ThemeMode)
    suspend fun loadAccentColor(): AccentColor
    suspend fun saveAccentColor(color: AccentColor)
}

class DataStoreUiPreferenceStorage(
    private val context: Context,
) : UiPreferenceStorage {

    override suspend fun loadThemeMode(): ThemeMode {
        return context.themeDataStore.data
            .map { preferences ->
                val savedName = preferences[KEY_THEME_MODE]
                savedName?.toThemeModeOrNull() ?: ThemeMode.Light
            }
            .first()
    }

    override suspend fun saveThemeMode(mode: ThemeMode) {
        context.themeDataStore.edit { preferences ->
            preferences[KEY_THEME_MODE] = mode.name
        }
    }

    override suspend fun loadAccentColor(): AccentColor {
        return context.themeDataStore.data
            .map { preferences ->
                val savedName = preferences[KEY_ACCENT_COLOR]
                savedName?.toAccentColorOrNull() ?: AccentColor.Blue
            }
            .first()
    }

    override suspend fun saveAccentColor(color: AccentColor) {
        context.themeDataStore.edit { preferences ->
            preferences[KEY_ACCENT_COLOR] = color.name
        }
    }

    private fun String.toThemeModeOrNull(): ThemeMode? {
        return ThemeMode.entries.firstOrNull { it.name == this }
    }

    private fun String.toAccentColorOrNull(): AccentColor? {
        return AccentColor.entries.firstOrNull { it.name == this }
    }

    private companion object {
        val KEY_THEME_MODE = stringPreferencesKey("theme_mode")
        val KEY_ACCENT_COLOR = stringPreferencesKey("accent_color")
    }
}
