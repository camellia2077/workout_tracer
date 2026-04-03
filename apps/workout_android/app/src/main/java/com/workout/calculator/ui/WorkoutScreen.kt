package com.workout.calculator.ui

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.RowScope
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.rounded.List
import androidx.compose.material.icons.automirrored.rounded.ManageSearch
import androidx.compose.material.icons.rounded.AddCircle
import androidx.compose.material.icons.rounded.Settings
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.Alignment
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.unit.dp
import com.workout.calculator.core.DisplayUnit
import com.workout.calculator.core.DisplayUnitFormatter
import com.workout.calculator.mvp.AccentColor
import com.workout.calculator.core.PersonalRecord
import com.workout.calculator.mvp.QueryType
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
    onDisplayUnitSelected: (DisplayUnit) -> Unit,
    onImportClick: () -> Unit,
    onImportArchiveClick: () -> Unit,
    onExportArchiveClick: () -> Unit,
    onClearDatabaseClick: () -> Unit,
    onClearTxtFilesClick: () -> Unit,
    onQueryClick: () -> Unit,
    onPrCardClick: (PersonalRecord) -> Unit,
    onMonthSelected: (String) -> Unit,
    onTypeSelected: (QueryType) -> Unit,
) {
    val pendingClearAction = remember { mutableStateOf<DataClearAction?>(null) }

    Scaffold(
        bottomBar = {
            NavigationBar(
                containerColor = MaterialTheme.colorScheme.surfaceContainer,
                tonalElevation = 0.dp,
            ) {
                TabItem(
                    label = "Data",
                    icon = Icons.Rounded.AddCircle,
                    selected = state.selectedTab == WorkoutTab.Data,
                    onClick = { onTabSelected(WorkoutTab.Data) },
                )
                TabItem(
                    label = "Query",
                    icon = Icons.AutoMirrored.Rounded.ManageSearch,
                    selected = state.selectedTab == WorkoutTab.Query,
                    onClick = { onTabSelected(WorkoutTab.Query) },
                )
                TabItem(
                    label = "Records",
                    icon = Icons.AutoMirrored.Rounded.List,
                    selected = state.selectedTab == WorkoutTab.Records,
                    onClick = { onTabSelected(WorkoutTab.Records) },
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
            WorkoutTab.Data -> InsertTabContent(
                state = state,
                onImportClick = onImportClick,
                onImportArchiveClick = onImportArchiveClick,
                onExportArchiveClick = onExportArchiveClick,
                onClearDatabaseClick = { pendingClearAction.value = DataClearAction.Database },
                onClearTxtFilesClick = { pendingClearAction.value = DataClearAction.TxtFiles },
                modifier = Modifier.padding(innerPadding),
            )

            WorkoutTab.Query -> QueryTabContent(
                state = state,
                onQueryClick = onQueryClick,
                onMonthSelected = onMonthSelected,
                onTypeSelected = onTypeSelected,
                modifier = Modifier.padding(innerPadding),
            )

            WorkoutTab.Records -> RecordsTabContent(
                state = state,
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
                selectedDisplayUnit = state.displayUnit,
                onDisplayUnitSelected = onDisplayUnitSelected,
                modifier = Modifier.padding(innerPadding),
            )
        }
    }

    val action = pendingClearAction.value
    if (action != null) {
        AlertDialog(
            onDismissRequest = { pendingClearAction.value = null },
            title = { Text(text = action.title) },
            text = { Text(text = action.message) },
            confirmButton = {
                TextButton(
                    onClick = {
                        when (action) {
                            DataClearAction.Database -> onClearDatabaseClick()
                            DataClearAction.TxtFiles -> onClearTxtFilesClick()
                        }
                        pendingClearAction.value = null
                    },
                ) {
                    Text(text = "Confirm")
                }
            },
            dismissButton = {
                TextButton(onClick = { pendingClearAction.value = null }) {
                    Text(text = "Cancel")
                }
            },
        )
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
    onImportClick: () -> Unit,
    onImportArchiveClick: () -> Unit,
    onExportArchiveClick: () -> Unit,
    onClearDatabaseClick: () -> Unit,
    onClearTxtFilesClick: () -> Unit,
    modifier: Modifier = Modifier,
) {
    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        Text(
            text = "Data",
            style = MaterialTheme.typography.headlineSmall,
        )
        Button(onClick = onImportClick) {
            Text(text = "Import TXT Folder")
        }
        Button(onClick = onImportArchiveClick) {
            Text(text = "Import Archive")
        }
        Button(onClick = onExportArchiveClick) {
            Text(text = "Export Archive")
        }
        OutlinedButton(onClick = onClearDatabaseClick) {
            Text(text = "Clear Database")
        }
        OutlinedButton(onClick = onClearTxtFilesClick) {
            Text(text = "Clear TXT Files")
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
    onMonthSelected: (String) -> Unit,
    onTypeSelected: (QueryType) -> Unit,
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
                text = "Month (${state.monthOptions.size})",
                style = MaterialTheme.typography.titleMedium,
                modifier = Modifier.padding(top = 8.dp, bottom = 4.dp),
            )
        }

        item {
            if (state.monthOptions.isEmpty()) {
                Text(
                    text = "No month data.",
                    style = MaterialTheme.typography.bodySmall,
                )
            } else {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .horizontalScroll(rememberScrollState()),
                    horizontalArrangement = Arrangement.spacedBy(8.dp),
                ) {
                    state.monthOptions.forEach { month ->
                        val selected = state.selectedMonth == month
                        if (selected) {
                            Button(onClick = { onMonthSelected(month) }) {
                                Text(text = month)
                            }
                        } else {
                            OutlinedButton(onClick = { onMonthSelected(month) }) {
                                Text(text = month)
                            }
                        }
                    }
                }
            }
        }

        item {
            Text(
                text = "Type",
                style = MaterialTheme.typography.titleMedium,
                modifier = Modifier.padding(top = 8.dp, bottom = 4.dp),
            )
        }

        item {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp),
            ) {
                QueryTypeButton(
                    type = QueryType.Squat,
                    selectedType = state.selectedType,
                    enabled = state.selectedMonth != null,
                    onTypeSelected = onTypeSelected,
                )
                QueryTypeButton(
                    type = QueryType.Pull,
                    selectedType = state.selectedType,
                    enabled = state.selectedMonth != null,
                    onTypeSelected = onTypeSelected,
                )
                QueryTypeButton(
                    type = QueryType.Push,
                    selectedType = state.selectedType,
                    enabled = state.selectedMonth != null,
                    onTypeSelected = onTypeSelected,
                )
            }
        }

        item {
            Text(
                text = "Report Markdown",
                style = MaterialTheme.typography.titleMedium,
                modifier = Modifier.padding(top = 8.dp, bottom = 4.dp),
            )
        }

        item {
            when {
                state.selectedMonth == null -> {
                    Text(
                        text = "Select a month first.",
                        style = MaterialTheme.typography.bodySmall,
                    )
                }

                state.isReportLoading -> {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.spacedBy(8.dp),
                    ) {
                        CircularProgressIndicator(modifier = Modifier.height(18.dp))
                        Text(
                            text = "Loading report preview for ${state.selectedMonth}/${state.selectedType.coreValue}...",
                            style = MaterialTheme.typography.bodySmall,
                        )
                    }
                }

                state.currentReportMarkdown.isBlank() -> {
                    Text(
                        text = "No report preview available.",
                        style = MaterialTheme.typography.bodySmall,
                    )
                }

                else -> RawMarkdownTextArea(markdown = state.currentReportMarkdown)
            }
        }
    }
}

@Composable
private fun RawMarkdownTextArea(
    markdown: String,
    modifier: Modifier = Modifier,
) {
    Card(modifier = modifier.fillMaxWidth()) {
        SelectionContainer {
            Text(
                text = markdown,
                style = MaterialTheme.typography.bodySmall,
                modifier = Modifier.padding(12.dp),
            )
        }
    }
}

@Composable
private fun RecordsTabContent(
    state: WorkoutUiState,
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
                text = "Records",
                style = MaterialTheme.typography.headlineSmall,
            )
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
                text = "PRs (${state.personalRecords.size})",
                style = MaterialTheme.typography.titleMedium,
                modifier = Modifier.padding(top = 8.dp, bottom = 4.dp),
            )
        }

        if (state.personalRecords.isEmpty()) {
            item {
                Text(
                    text = "No records yet. Run Query first.",
                    style = MaterialTheme.typography.bodySmall,
                )
            }
        }

        items(state.personalRecords) { pr ->
            val prKey = buildPrKey(pr)
            val isExpanded = state.expandedPrKey == prKey
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { onPrCardClick(pr) }
            ) {
                Column(modifier = Modifier.padding(12.dp)) {
                    Text(text = pr.exerciseName)
                    val maxWeightText = DisplayUnitFormatter.formatDetailWeight(
                        weightKg = pr.maxWeightKg,
                        originalUnit = pr.originalUnit,
                        originalWeightValue = pr.originalWeightValue,
                        displayUnit = state.displayUnit,
                    )
                    Text(
                        text = "max=$maxWeightText reps=${pr.reps} date=${pr.date}",
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
    selectedDisplayUnit: DisplayUnit,
    onDisplayUnitSelected: (DisplayUnit) -> Unit,
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
                Text(
                    text = "Display Unit",
                    style = MaterialTheme.typography.titleSmall,
                    modifier = Modifier.padding(top = 8.dp, bottom = 4.dp),
                )
                DisplayUnitOptionRow(
                    label = "Original",
                    selected = selectedDisplayUnit == DisplayUnit.Original,
                    onClick = { onDisplayUnitSelected(DisplayUnit.Original) },
                )
                DisplayUnitOptionRow(
                    label = "KG",
                    selected = selectedDisplayUnit == DisplayUnit.Kg,
                    onClick = { onDisplayUnitSelected(DisplayUnit.Kg) },
                )
                DisplayUnitOptionRow(
                    label = "LB",
                    selected = selectedDisplayUnit == DisplayUnit.Lb,
                    onClick = { onDisplayUnitSelected(DisplayUnit.Lb) },
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
private fun QueryTypeButton(
    type: QueryType,
    selectedType: QueryType,
    enabled: Boolean,
    onTypeSelected: (QueryType) -> Unit,
) {
    val selected = type == selectedType
    val text = type.coreValue
    if (selected) {
        Button(
            onClick = { onTypeSelected(type) },
            enabled = enabled,
        ) {
            Text(text = text)
        }
    } else {
        OutlinedButton(
            onClick = { onTypeSelected(type) },
            enabled = enabled,
        ) {
            Text(text = text)
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
private fun DisplayUnitOptionRow(
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

private fun buildPrKey(record: PersonalRecord): String {
    return buildString {
        append(record.exerciseName)
        append('|')
        append(record.date)
        append('|')
        append(record.reps)
        append('|')
        append(record.maxWeightKg)
        append('|')
        append(record.originalUnit)
        append('|')
        append(record.originalWeightValue)
    }
}

private enum class DataClearAction(
    val title: String,
    val message: String,
) {
    Database(
        title = "Clear Database?",
        message = "This will delete all local database files and cannot be undone.",
    ),
    TxtFiles(
        title = "Clear TXT Files?",
        message = "This will delete imported local TXT snapshots and cannot be undone.",
    ),
}
