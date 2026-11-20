#!/usr/bin/env bash
set -euo pipefail

ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-}}"
if [[ -z "$ANDROID_SDK_ROOT" || ! -d "$ANDROID_SDK_ROOT" ]]; then
    echo "ERROR: Android SDK not found. Set ANDROID_SDK_ROOT or ANDROID_HOME." >&2
    exit 1
fi

NDK_VERSION="${NDK_VERSION:-26.1.10909125}"
BUILD_TOOLS_VERSION="${BUILD_TOOLS_VERSION:-34.0.0}"
ANDROID_PLATFORM="${ANDROID_PLATFORM:-34}"
ANDROID_NDK="${ANDROID_NDK:-$ANDROID_SDK_ROOT/ndk/$NDK_VERSION}"
BUILD_TOOLS="$ANDROID_SDK_ROOT/build-tools/$BUILD_TOOLS_VERSION"
PLATFORM="$ANDROID_SDK_ROOT/platforms/android-$ANDROID_PLATFORM/android.jar"
TOOLCHAIN="$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin"
CC="$TOOLCHAIN/aarch64-linux-android24-clang"
GLUE_DIR="$ANDROID_NDK/sources/android/native_app_glue"

for required in "$CC" "$BUILD_TOOLS/aapt" "$BUILD_TOOLS/zipalign" "$BUILD_TOOLS/apksigner" "$PLATFORM" "$GLUE_DIR/android_native_app_glue.c"; do
    if [[ ! -e "$required" ]]; then
        echo "ERROR: Required Android build component not found: $required" >&2
        exit 1
    fi
done

command -v zip >/dev/null 2>&1 || { echo "ERROR: zip is not installed." >&2; exit 1; }
command -v keytool >/dev/null 2>&1 || { echo "ERROR: keytool is not available. Install a JDK." >&2; exit 1; }
command -v glslc >/dev/null 2>&1 || { echo "ERROR: glslc is required to compile Vulkan shaders." >&2; exit 1; }

echo "1. Compiling Vulkan shaders..."
glslc assets/shaders/menu.vert -o assets/shaders/menu.vert.spv
glslc assets/shaders/menu.frag -o assets/shaders/menu.frag.spv
glslc assets/shaders/sea.vert -o assets/shaders/sea.vert.spv
glslc assets/shaders/sea.frag -o assets/shaders/sea.frag.spv

mkdir -p build/lib/arm64-v8a
rm -f build/*.apk

echo "2. Compiling Tornia-guld native C code for Android arm64-v8a..."
"$CC" -Wall -Wextra -shared -fPIC \
    -I"$GLUE_DIR" \
    -I./include \
    src/*.c \
    "$GLUE_DIR/android_native_app_glue.c" \
    -o build/lib/arm64-v8a/libgame.so \
    -u ANativeActivity_onCreate \
    -landroid -llog -lvulkan -lm

echo "3. Packaging Android manifest and assets..."
"$BUILD_TOOLS/aapt" package -f -M AndroidManifest.xml \
    -I "$PLATFORM" \
    -A assets \
    -F build/unaligned.apk

(
    cd build
    zip -q -r unaligned.apk lib/
)

echo "4. Aligning and signing APK..."
"$BUILD_TOOLS/zipalign" -f 4 build/unaligned.apk build/aligned.apk

if [[ ! -f debug.keystore ]]; then
    keytool -genkey -v -keystore debug.keystore -storepass android \
        -alias androiddebugkey -keypass android -keyalg RSA \
        -keysize 2048 -validity 10000 \
        -dname "CN=Android Debug,O=Android,C=US"
fi

"$BUILD_TOOLS/apksigner" sign --ks debug.keystore \
    --ks-pass pass:android --key-pass pass:android \
    --out build/Tornia-guld-debug.apk build/aligned.apk
"$BUILD_TOOLS/apksigner" verify --verbose build/Tornia-guld-debug.apk
"$BUILD_TOOLS/zipalign" -c -v 4 build/Tornia-guld-debug.apk

echo "SUCCESS! APK ready at: build/Tornia-guld-debug.apk"
