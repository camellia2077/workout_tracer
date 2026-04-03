# UI 偏好持久化说明（Android）

## 目标
- 需要记住用户在 UI 里的偏好选项（例如主题：浅色 / 深色 / 跟随系统）。

## 存储方式
- 使用 `Jetpack DataStore (Preferences)` 存储。
- 不使用 `SharedPreferences` 作为新功能默认方案。

## 存储位置
- DataStore 名称：`workout_user_prefs`
- 设备落盘文件（应用私有目录）：
  - `/data/data/com.workout.calculator/files/datastore/workout_user_prefs.preferences_pb`

## 当前已使用的键
- `theme_mode`：保存主题模式（`Light` / `Dark` / `System`）。
- `accent_color`：保存主题主色（`Red` / `Orange` / `Yellow` / `Green` / `Cyan` / `Blue` / `Purple`）。
- `display_unit`：保存重量显示偏好（`Original` / `Kg` / `Lb`）。

## 代码位置
- `apps/workout_android/app/src/main/java/com/workout/calculator/mvp/ThemeModeStorage.kt`
