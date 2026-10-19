-keep class org.qtproject.qt.android.** { *; }
-keepclassmembers class org.qtproject.qt.android.** { *; }
-keep class org.kde.ausdroid.** { *; }
# Keep AndroidX FileProvider so it doesn't get stripped by minification
-keep class androidx.core.** { *; }
-keep public class androidx.core.content.FileProvider { *; }

# Keep native methods
-keepclassmembers class * {
    native <methods>;
}