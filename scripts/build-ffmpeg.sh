#!/bin/bash
# Cross-compile FFmpeg + LAME for iOS arm64 using Theos toolchain
# Usage: bash scripts/build-ffmpeg.sh [output_dir]
set -e

OUTPUT_DIR="${1:-$(pwd)/vendor/ffmpeg-ios}"
THEOS="${THEOS:-$HOME/theos}"
SDK="${THEOS}/sdks/iPhoneOS16.5.sdk"
TOOLCHAIN="${THEOS}/toolchain/linux/iphone/bin"
MIN_IOS="15.0"
NPROC=$(nproc)

FFMPEG_VERSION="7.1.1"
LAME_VERSION="3.100"

echo "=== FFmpeg/LAME iOS cross-compile ==="
echo "THEOS: $THEOS"
echo "SDK: $SDK"
echo "Output: $OUTPUT_DIR"

# Check toolchain
if [ ! -f "$TOOLCHAIN/clang" ]; then
    echo "ERROR: clang not found at $TOOLCHAIN/clang"
    echo "Install Theos first: git clone --recursive https://github.com/theos/theos.git $THEOS"
    exit 1
fi

if [ ! -d "$SDK" ]; then
    echo "ERROR: SDK not found at $SDK"
    exit 1
fi

mkdir -p "$OUTPUT_DIR/include" "$OUTPUT_DIR/lib"
BUILD_DIR=$(mktemp -d)
trap "rm -rf $BUILD_DIR" EXIT

# --- LAME ---
echo ""
echo "=== Building LAME $LAME_VERSION ==="
cd "$BUILD_DIR"
curl -L -o lame.tar.gz "https://downloads.sourceforge.net/project/lame/lame/${LAME_VERSION}/lame-${LAME_VERSION}.tar.gz"
tar xzf lame.tar.gz
cd "lame-${LAME_VERSION}"

HOSTCC=gcc \
CC="$TOOLCHAIN/clang" \
CFLAGS="-arch arm64 -isysroot $SDK -miphoneos-version-min=$MIN_IOS -O3" \
LDFLAGS="-arch arm64 -isysroot $SDK -miphoneos-version-min=$MIN_IOS" \
./configure \
    --host=arm-apple-darwin \
    --enable-static \
    --disable-shared \
    --disable-frontend \
    --disable-decoder \
    --prefix="$OUTPUT_DIR"

make -j$NPROC
make install
echo "LAME done: $(ls $OUTPUT_DIR/lib/libmp3lame.a)"

# --- FFmpeg ---
echo ""
echo "=== Building FFmpeg $FFMPEG_VERSION ==="
cd "$BUILD_DIR"
curl -L -o ffmpeg.tar.bz2 "https://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.bz2"
tar xjf ffmpeg.tar.bz2
cd "ffmpeg-${FFMPEG_VERSION}"

./configure \
    --enable-cross-compile \
    --arch=arm64 \
    --target-os=darwin \
    --cc="$TOOLCHAIN/clang" \
    --as="$TOOLCHAIN/clang" \
    --sysroot="$SDK" \
    --extra-cflags="-arch arm64 -miphoneos-version-min=$MIN_IOS -O3 -I$OUTPUT_DIR/include" \
    --extra-ldflags="-arch arm64 -miphoneos-version-min=$MIN_IOS -L$OUTPUT_DIR/lib" \
    --enable-static \
    --disable-shared \
    --disable-programs \
    --disable-doc \
    --disable-avdevice \
    --disable-swscale \
    --disable-postproc \
    --disable-avfilter \
    --disable-network \
    --disable-everything \
    --enable-muxer=mp4,mp3,wav,flac,ogg,matroska,adts \
    --enable-demuxer=mov,mp3,wav,flac,ogg,matroska,hls,mpegts,aac,adts \
    --enable-decoder=h264,hevc,aac,mp3,flac,vorbis,opus,alac \
    --enable-encoder=aac,libmp3lame,flac,opus,alac \
    --enable-protocol=file \
    --enable-libmp3lame \
    --enable-neon \
    --prefix="$OUTPUT_DIR"

make -j$NPROC
make install

echo ""
echo "=== Done ==="
echo "FFmpeg libs:"
ls -la "$OUTPUT_DIR/lib/"*.a
echo "Headers:"
ls "$OUTPUT_DIR/include/"
echo "Output: $OUTPUT_DIR"
