# qiperf
qt base app to lunch iperf2/3 in server mode


# build apk

```
# For Qt 6 Android builds, JDK = 17 or 21?
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64
export ANDROID_NDK_ROOT=/home/jimmy/bin/android/ndk/
export ANDROID_NDK_ROOT="/mnt/data/download/android/sdk/ndk/27.2.12479018"
export QT_ANDROID_KEYSTORE_PATH="$HOME/android_release.keystore.jks"
export QT_ANDROID_KEYSTORE_ALIAS="play"
export QT_ANDROID_KEYSTORE_STORE_PASS="your_store_password"

# 1. 建立並進入編譯資料夾（保持原始碼乾淨）
mkdir build-android && cd build-android
# 2. 執行對應 arm64-v8a 的 qmake
/opt/Qt/6.10.3/android_arm64_v8a/bin/qmake ../qiperf.pro -spec android-clang CONFIG+=release
# 3. 開始編譯
cd ../iperf3 ; make clean ; cd ../lksctp ; make clean
cd ../build-android ; make clean ; make -j$(nproc)

# build debug apk/aab
make aab 

ANDROID_DEPLOYMENT_SETTINGS=android-qiperf-deployment-settings.json \
         ANDROID_KEYSTORE_PATH==/home/coolshou/android_release.keystore.jks \
         ANDROID_KEYSTORE_ALIAS=play

# install debug apk
adb install -r android-build/build/outputs/apk/debug/android-build-debug.apk

# uninstall 
adb uninstall tw.idv.coolshou.qiperf

# 4. build release apk/aab
/opt/Qt/6.10.3/gcc_64/bin/androiddeployqt6 \
    --input android-qiperf-deployment-settings.json \
    --output android-build \
    --aab \
    --release \
    --sign /home/coolshou/android_release.keystore.jks play \
    --storepass "your_keystore_password"

# install signed apk
adb install -r android-build/build/outputs/apk/release/android-build-release-signed.apk

```


產生的簽署版 APK 將會儲存在建構目錄下的 android-build/build/outputs/apk/release/