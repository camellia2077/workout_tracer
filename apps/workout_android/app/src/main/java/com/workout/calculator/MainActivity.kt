package com.workout.calculator

import android.os.Bundle
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.remember
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.workout.calculator.core.WorkoutNativeBridge
import com.workout.calculator.mvp.DataStoreUiPreferenceStorage
import com.workout.calculator.mvp.NativeWorkoutRepository
import com.workout.calculator.mvp.WorkoutContract
import com.workout.calculator.mvp.ThemeMode
import com.workout.calculator.mvp.WorkoutPresenter
import com.workout.calculator.ui.accentPaletteFor
import com.workout.calculator.ui.WorkoutScreen

class MainActivity : ComponentActivity() {
    private lateinit var presenter: WorkoutContract.Presenter

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        presenter = WorkoutPresenter(
            repository = NativeWorkoutRepository(applicationContext),
            preferenceStorage = DataStoreUiPreferenceStorage(applicationContext),
        )

        setContent {
            val state = presenter.state.collectAsStateWithLifecycle().value
            val presentationVersion = remember { resolvePresentationVersion() }
            val coreVersion = remember {
                runCatching { WorkoutNativeBridge.getCoreVersion() }
                    .getOrDefault("unknown")
            }
            val isSystemDark = isSystemInDarkTheme()
            val useDarkTheme = when (state.themeMode) {
                ThemeMode.Light -> false
                ThemeMode.Dark -> true
                ThemeMode.System -> isSystemDark
            }
            val accentPalette = accentPaletteFor(state.accentColor)
            val baseScheme = if (useDarkTheme) darkColorScheme() else lightColorScheme()
            val colorScheme = if (useDarkTheme) {
                baseScheme.copy(
                    primary = accentPalette.darkPrimary,
                    secondary = accentPalette.darkPrimary,
                    tertiary = accentPalette.darkPrimary,
                    primaryContainer = accentPalette.darkPrimaryContainer,
                    secondaryContainer = accentPalette.darkPrimaryContainer,
                    tertiaryContainer = accentPalette.darkPrimaryContainer,
                )
            } else {
                baseScheme.copy(
                    primary = accentPalette.lightPrimary,
                    secondary = accentPalette.lightPrimary,
                    tertiary = accentPalette.lightPrimary,
                    primaryContainer = accentPalette.lightPrimaryContainer,
                    secondaryContainer = accentPalette.lightPrimaryContainer,
                    tertiaryContainer = accentPalette.lightPrimaryContainer,
                )
            }
            MaterialTheme(
                colorScheme = colorScheme
            ) {
                val folderPickerLauncher = rememberLauncherForActivityResult(
                    contract = ActivityResultContracts.OpenDocumentTree(),
                ) { uri ->
                    if (uri != null) {
                        presenter.onImportFolderSelected(uri.toString())
                    }
                }
                val archiveImportLauncher = rememberLauncherForActivityResult(
                    contract = ActivityResultContracts.OpenDocument(),
                ) { uri ->
                    if (uri != null) {
                        presenter.onImportArchiveSelected(uri.toString())
                    }
                }
                val archiveExportLauncher = rememberLauncherForActivityResult(
                    contract = ActivityResultContracts.CreateDocument("application/zip"),
                ) { uri ->
                    if (uri != null) {
                        presenter.onExportArchiveSelected(uri.toString())
                    }
                }
                Surface {
                    WorkoutScreen(
                        state = state,
                        presentationVersion = presentationVersion,
                        coreVersion = coreVersion,
                        onTabSelected = presenter::onTabSelected,
                        onThemeModeSelected = presenter::onThemeModeSelected,
                        onAccentColorSelected = presenter::onAccentColorSelected,
                        onDisplayUnitSelected = presenter::onDisplayUnitSelected,
                        onImportClick = { folderPickerLauncher.launch(null) },
                        onImportArchiveClick = {
                            archiveImportLauncher.launch(arrayOf("application/zip"))
                        },
                        onExportArchiveClick = {
                            archiveExportLauncher.launch("workout_backup.zip")
                        },
                        onClearDatabaseClick = presenter::onClearDatabaseClick,
                        onClearTxtFilesClick = presenter::onClearTxtFilesClick,
                        onQueryClick = presenter::onQueryClick,
                        onPrCardClick = presenter::onPrCardClick,
                        onMonthSelected = presenter::onMonthSelected,
                        onTypeSelected = presenter::onTypeSelected,
                    )
                }
            }
        }
    }

    override fun onDestroy() {
        presenter.dispose()
        super.onDestroy()
    }

    @Suppress("DEPRECATION")
    private fun resolvePresentationVersion(): String {
        return runCatching {
            packageManager.getPackageInfo(packageName, 0).versionName ?: "unknown"
        }.getOrDefault("unknown")
    }
}
