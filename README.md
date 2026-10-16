# qiperf
qt base app to lunch iperf2/3 in server mode


# build apk

```
export ANDROID_NDK_ROOT=/home/jimmy/bin/android/ndk/
# 1. 建立並進入編譯資料夾（保持原始碼乾淨）
mkdir build-android && cd build-android
# 2. 執行對應 arm64-v8a 的 qmake
/mnt/SOFT/software/dev/QT/src/6.10.3/android_arm64_v8a/bin/qmake ../qiperf.pro -spec android-clang CONFIG+=release
# 3. 開始編譯並建構 APK
make -j$(nproc)
make apk ANDROID_DEPLOYMENT_SETTINGS=android-your_project-deployment-settings.json \
         ANDROID_KEYSTORE_PATH=/<path_to_keystore>/android_release.keystore.jks \
         ANDROID_KEYSTORE_ALIAS=play

         ANDROID_KEYSTORE_STORE_PASS=您的金鑰庫密碼 \
         ANDROID_KEYSTORE_KEY_PASS=您的別名密碼
# build release apk
cd android-build
./gradlew assembleRelease  \
-Pandroid.injected.signing.store.file=/<path_to_keystore>/android_release.keystore.jks \
-Pandroid.injected.signing.key.alias=play \
-Pandroid.injected.signing.store.password=您的金鑰庫密碼 \
-Pandroid.injected.signing.key.password=您的別名密碼
```


產生的簽署版 APK 將會儲存在建構目錄下的 android-build/build/outputs/apk/release/