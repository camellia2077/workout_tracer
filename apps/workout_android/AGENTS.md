# AGENTS.md

## Scope
This file applies to `apps/workout_android` and all its subdirectories.

## Android Validation Policy
- For Android-related changes, validate by Android build tasks only.
- Do not run repository-wide C++ configure/build/test flows.
- Do not run Windows CLI related verification (for example `workout_tracker_cli` or other `windows_cli` checks), unless explicitly requested.

## Required Checks After Android Changes
- Prefer root-level Python entrypoints:
  - Default: `python tools/run.py android assemble-debug`
  - Release APK: `python tools/run.py android assemble-release`
  - JNI/CMake changes under Android app: `python tools/run.py android native-debug`
  - Release JNI/CMake changes: `python tools/run.py android native-release`
  - Android unit tests when explicitly needed: `python tools/run.py android test-debug`
- Direct Gradle equivalents:
  - Default: `apps/workout_android/gradlew.bat :app:assembleDebug`
  - Release APK: `apps/workout_android/gradlew.bat :app:assembleRelease`
  - Kotlin/Compose-only changes: `apps/workout_android/gradlew.bat :app:compileDebugKotlin`
  - Resource-only changes: `apps/workout_android/gradlew.bat :app:processDebugResources`
  - JNI/CMake changes under Android app: `apps/workout_android/gradlew.bat :app:externalNativeBuildDebug`
  - Release JNI/CMake changes: `apps/workout_android/gradlew.bat :app:externalNativeBuildRelease`

## Network / Proxy (Windows PowerShell)
- If build fails with TLS/SSL handshake or cannot resolve Google Maven, configure proxy before Android builds.
- Current session proxy:
  - `$env:HTTP_PROXY  = "http://127.0.0.1:9910"`
  - `$env:HTTPS_PROXY = "http://127.0.0.1:9910"`
- Persistent Gradle proxy (user-level): append to `C:\Users\17596\.gradle\gradle.properties`
  - `systemProp.http.proxyHost=127.0.0.1`
  - `systemProp.http.proxyPort=9910`
  - `systemProp.https.proxyHost=127.0.0.1`
  - `systemProp.https.proxyPort=9910`
  - `systemProp.http.nonProxyHosts=localhost|127.0.0.1`
  - `systemProp.https.nonProxyHosts=localhost|127.0.0.1`
- Validate connectivity:
  - `curl -x http://127.0.0.1:9910 -I https://dl.google.com/dl/android/maven2/`
- Restart daemon before retry:
  - `apps/workout_android/gradlew.bat --stop`

## Reference Docs
- UI preference persistence guide:
  - `docs/workout_android/ui_preferences_storage.md`

## Notes
- Only run additional tests when the user explicitly asks for them.
