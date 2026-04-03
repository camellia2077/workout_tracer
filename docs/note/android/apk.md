## debug

python tools/run.py android assemble-debug


对应 Gradle 任务 :app:assembleDebug
会产出完整的 debug apk
它通常会连带走完 Android app 需要的整套 debug 构建流程，包括 Kotlin/Java、资源、manifest、DEX、JNI/native 产物打包等


##
python tools/run.py android native-debug

对应 Gradle 任务 :app:externalNativeBuildDebug
只编 Android 的 JNI/C++ native 部分
不会产出完整 APK


# test
python tools/run.py android test-debug

