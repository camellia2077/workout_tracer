# Keep JNI entrypoints stable for name-based native lookup.
-keep class com.workout.calculator.core.WorkoutNativeBridge { *; }

# Keep native bridge result model for JNI FindClass/GetMethodID.
-keep class com.workout.calculator.core.NativeResult { *; }
