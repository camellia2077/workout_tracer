package com.workout.calculator.ui

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.RowScope
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.rounded.ManageSearch
import androidx.compose.material.icons.rounded.AddCircle
import androidx.compose.material.icons.rounded.Settings
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.Alignment
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.unit.dp
import com.workout.calculator.mvp.AccentColor
import com.workout.calculator.core.ExerciseRecord
import com.workout.calculator.core.PersonalRecord
import com.workout.calculator.mvp.ThemeMode
import com.workout.calculator.mvp.WorkoutTab
import com.workout.calculator.mvp.WorkoutUiState

@Composable
fun WorkoutScreen(
    state: WorkoutUiState,
    presentationVersion: String,
    coreVersion: String,
    onTabSelected: (WorkoutTab) -> Unit,
    onThemeModeSelected: (ThemeMode) -> Unit,
    onAccentColorSelected: (AccentColor) -> Unit,
    onIngestClick: () -> Unit,
    onQueryClick: () -> Unit,
    onExerciseCardClick: (ExerciseRecord) -> Unit,
    onPrCardClick: (PersonalRecord) -> Unit,
) {
    Scaffold(
        bottomBar = {
            NavigationBar(
                containerColor = MaterialTheme.colorScheme.surfaceContainer,
                tonalElevation = 0.dp,
            ) {
                TabItem(
                    label = "Insert",
                    icon = Icons.Rounded.AddCircle,
                    selected = state.selectedTab == WorkoutTab.Insert,
                    onClick = { onTabSelected(WorkoutTab.Insert) },
                )
                TabItem(
                    label = "Query",
                    icon = Icons.AutoMirrored.Rounded.ManageSearch,
                    selected = state.selectedTab == WorkoutTab.Query,
                    onClick = { onTabSelected(WorkoutTab.Query) },
                )
                TabItem(
                    label = "Config",
                    icon = Icons.Rounded.Settings,
                    selected = state.selectedTab == WorkoutTab.Config,
                    onClick = { onTabSelected(WorkoutTab.Config) },
                )
            }
        }
    ) { innerPadding ->
        when (state.selectedTab) {
            WorkoutTab.Insert -> InsertTabContent(
                state = state,
                onIngestClick = onIngestClick,
                modifier = Modifier.padding(innerPadding),
            )

            WorkoutTab.Query -> QueryTabContent(
                state = state,
                onQueryClick = onQueryClick,
                onExerciseCardClick = onExerciseCardClick,
                onPrCardClick = onPrCardClick,
                modifier = Modifier.padding(innerPadding),
            )

            WorkoutTab.Config -> ConfigTabContent(
                presentationVersion = presentationVersion,
                coreVersion = coreVersion,
                selectedThemeMode = state.themeMode,
                onThemeModeSelected = onThemeModeSelected,
                selectedAccentColor = state.accentColor,
                onAccentColorSelected = onAccentColorSelected,
                modifier = Modifier.padding(innerPadding),
            )
        }
    }
}

@Composable
private fun RowScope.TabItem(
    label: String,
    icon: ImageVector,
    selected: Boolean,
    onClick: () -> Unit,
) {
    val contentColor = if (selected) {
        MaterialTheme.colorScheme.primary
    } else {
        MaterialTheme.colorScheme.onSurfaceVariant
    }
    Column(
        modifier = Modifier
            .weight(1f)
            .clickable(onClick = onClick)
            .padding(vertical = 10.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(2.dp),
    ) {
        Icon(
            imageVector = icon,
            contentDescription = label,
            tint = contentColor,
        )
        Text(
            text = label,
            style = MaterialTheme.typography.labelMedium,
            color = contentColor,
        )
    }
}

@Composable
private fun InsertTabContent(
    state: WorkoutUiState,
    onIngestClick: () -> Unit,
    modifier: Modifier = Modifier,
) {
    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        Text(
            text = "Insert",
            style = MaterialTheme.typography.headlineSmall,
        )
        Button(onClick = onIngestClick) {
            Text(text = "Ingest test/data/records/2025_7")
        }
        if (state.isLoading) {
            CircularProgressIndicator()
        }
        Text(
            text = state.statusText,
            style = MaterialTheme.typography.bodySmall,
        )
    }
}

@Composable
private fun QueryTabContent(
    state: WorkoutUiState,
    onQueryClick: () -> Unit,
    onExerciseCardClick: (ExerciseRecord) -> Unit,
    onPrCardClick: (PersonalRecord) -> Unit,
    modifier: Modifier = Modifier,
) {
    LazyColumn(
        modifier = modifier.fillMaxSize(),
        contentPadding = PaddingValues(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        item {
        Text(
            text = "Query",
            style = MaterialTheme.typography.headlineSmall,
        )
        }

        item {
            Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                Button(onClick = onQueryClick) {
                    Text(text = "Query")
                }
            }
        }

        if (state.isLoading) {
            item {
                CircularProgressIndicator()
            }
        }

        item {
            Text(
                text = state.statusText,
                style = MaterialTheme.typography.bodySmall,
            )
        }

        item {
            Text(
                text = "Exercises (${state.exercises.size})",
                style = MaterialTheme.typography.titleMedium,
            )
        }

        items(state.exercises) { exercise ->
            val isExpanded = state.expandedExerciseName == exercise.name
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { onExerciseCardClick(exercise) }
            ) {
                Column(modifier = Modifier.padding(12.dp)) {
                    Text(text = "${exercise.name} [${exercise.type}]")
                    if (isExpanded) {
                        Text(
                            text = "Markdown",
                            style = MaterialTheme.typography.labelMedium,
                            modifier = Modifier.padding(top = 8.dp, bottom = 4.dp),
                        )
                        MarkdownContent(markdown = state.expandedExerciseMarkdown)
                    }
                }
            }
        }

        item {
            Text(
                text = "PRs (${state.personalRecords.size})",
                style = MaterialTheme.typography.titleMedium,
                modifier = Modifier.padding(top = 8.dp, bottom = 4.dp),
            )
        }

        items(state.personalRecords) { pr ->
            val prKey = "${pr.exerciseName}|${pr.date}|${pr.reps}|${pr.maxWeight}"
            val isExpanded = state.expandedPrKey == prKey
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { onPrCardClick(pr) }
            ) {
                Column(modifier = Modifier.padding(12.dp)) {
                    Text(text = pr.exerciseName)
                    Text(
                        text = "max=${pr.maxWeight} reps=${pr.reps} date=${pr.date}",
                        style = MaterialTheme.typography.bodySmall,
                    )
                    if (isExpanded) {
                        Text(
                            text = "Markdown",
                            style = MaterialTheme.typography.labelMedium,
                            modifier = Modifier.padding(top = 8.dp, bottom = 4.dp),
                        )
                        MarkdownContent(markdown = state.expandedPrMarkdown)
                    }
                }
            }
        }
    }
}

@Composable
private fun ConfigTabContent(
    presentationVersion: String,
    coreVersion: String,
    selectedThemeMode: ThemeMode,
    onThemeModeSelected: (ThemeMode) -> Unit,
    selectedAccentColor: AccentColor,
    onAccentColorSelected: (AccentColor) -> Unit,
    modifier: Modifier = Modifier,
) {
    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        Text(
            text = "Config",
            style = MaterialTheme.typography.headlineSmall,
        )
        Card(modifier = Modifier.fillMaxWidth()) {
            Column(modifier = Modifier.padding(12.dp)) {
                Text(
                    text = "Appearance",
                    style = MaterialTheme.typography.titleSmall,
                    modifier = Modifier.padding(bottom = 4.dp),
                )
                ThemeModeOptionRow(
                    label = "Light",
                    selected = selectedThemeMode == ThemeMode.Light,
                    onClick = { onThemeModeSelected(ThemeMode.Light) },
                )
                ThemeModeOptionRow(
                    label = "Dark",
                    selected = selectedThemeMode == ThemeMode.Dark,
                    onClick = { onThemeModeSelected(ThemeMode.Dark) },
                )
                ThemeModeOptionRow(
                    label = "Follow System",
                    selected = selectedThemeMode == ThemeMode.System,
                    onClick = { onThemeModeSelected(ThemeMode.System) },
                )
                Text(
                    text = "Accent Color (Rainbow Order)",
                    style = MaterialTheme.typography.titleSmall,
                    modifier = Modifier.padding(top = 8.dp, bottom = 4.dp),
                )
                AccentColorOptionRow(
                    color = AccentColor.Red,
                    selected = selectedAccentColor == AccentColor.Red,
                    onClick = { onAccentColorSelected(AccentColor.Red) },
                )
                AccentColorOptionRow(
                    color = AccentColor.Orange,
                    selected = selectedAccentColor == AccentColor.Orange,
                    onClick = { onAccentColorSelected(AccentColor.Orange) },
                )
                AccentColorOptionRow(
                    color = AccentColor.Yellow,
                    selected = selectedAccentColor == AccentColor.Yellow,
                    onClick = { onAccentColorSelected(AccentColor.Yellow) },
                )
                AccentColorOptionRow(
                    color = AccentColor.Green,
                    selected = selectedAccentColor == AccentColor.Green,
                    onClick = { onAccentColorSelected(AccentColor.Green) },
                )
                AccentColorOptionRow(
                    color = AccentColor.Cyan,
                    selected = selectedAccentColor == AccentColor.Cyan,
                    onClick = { onAccentColorSelected(AccentColor.Cyan) },
                )
                AccentColorOptionRow(
                    color = AccentColor.Blue,
                    selected = selectedAccentColor == AccentColor.Blue,
                    onClick = { onAccentColorSelected(AccentColor.Blue) },
                )
                AccentColorOptionRow(
                    color = AccentColor.Purple,
                    selected = selectedAccentColor == AccentColor.Purple,
                    onClick = { onAccentColorSelected(AccentColor.Purple) },
                )
            }
        }
        Card(modifier = Modifier.fillMaxWidth()) {
            Column(modifier = Modifier.padding(12.dp)) {
                Text(
                    text = "About",
                    style = MaterialTheme.typography.titleSmall,
                    modifier = Modifier.padding(bottom = 4.dp),
                )
                Text(text = "CPP Core Version: $coreVersion")
                Text(text = "Android Presentation Version: $presentationVersion")
                Text(text = "Author: camellia2077")
                Text(text = "Project: https://github.com/camellia2077/workout_calculator")
            }
        }
    }
}

@Composable
private fun ThemeModeOptionRow(
    label: String,
    selected: Boolean,
    onClick: () -> Unit,
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .padding(vertical = 2.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        RadioButton(
            selected = selected,
            onClick = onClick,
        )
        Text(
            text = label,
            style = MaterialTheme.typography.bodyMedium,
            modifier = Modifier.padding(start = 4.dp),
        )
    }
}

@Composable
private fun AccentColorOptionRow(
    color: AccentColor,
    selected: Boolean,
    onClick: () -> Unit,
) {
    val preview = accentPaletteFor(color).lightPrimary
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .padding(vertical = 2.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        RadioButton(
            selected = selected,
            onClick = onClick,
        )
        Text(
            text = "●",
            color = preview,
            style = MaterialTheme.typography.bodyLarge,
            modifier = Modifier.padding(start = 4.dp),
        )
        Text(
            text = accentColorLabel(color),
            style = MaterialTheme.typography.bodyMedium,
            modifier = Modifier.padding(start = 8.dp),
        )
    }
}
