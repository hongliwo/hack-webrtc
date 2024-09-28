#!/bin/bash

set -e

export PATH=/home/ubuntu/project/hackwebrtc/src/third_party/llvm-build/Release+Asserts/bin:$PATH

#PY=/home/ubuntu/project/hackwebrtc/depot_tools/python2-bin/python2
#PY=/home/ubuntu/project/hackwebrtc/src/third_party/depot_tools/python2-bin/python2
PY=python2

pushd third_party/ffmpeg

echo "echo "This is line $LINENO""

git reset --hard
git apply ../../sdk/ffmpeg-others-build.diff

rm -rf build.x64.linux

$PY chromium/scripts/build_ffmpeg.py android arm-neon --branding Chrome -- \
    --disable-asm \
    --disable-encoders --disable-hwaccels --disable-bsfs --disable-devices --disable-filters \
    --disable-protocols --enable-protocol=file \
    --disable-parsers --enable-parser=mpegaudio --enable-parser=h264 --enable-parser=hevc \
    --disable-demuxers --enable-demuxer=mov --enable-demuxer=mp3 --enable-demuxer=mpegts \
    --disable-decoders --enable-decoder=mp3 --enable-decoder=aac \
    --disable-muxers --enable-muxer=matroska \
    --enable-swresample

$PY chromium/scripts/build_ffmpeg.py android ia32 --branding Chrome -- \
    --disable-asm \
    --disable-encoders --disable-hwaccels --disable-bsfs --disable-devices --disable-filters \
    --disable-protocols --enable-protocol=file \
    --disable-parsers --enable-parser=mpegaudio --enable-parser=h264 --enable-parser=hevc \
    --disable-demuxers --enable-demuxer=mov --enable-demuxer=mp3 --enable-demuxer=mpegts \
    --disable-decoders --enable-decoder=mp3 --enable-decoder=aac \
    --disable-muxers --enable-muxer=matroska \
    --enable-swresample

$PY chromium/scripts/build_ffmpeg.py android arm64 --branding Chrome -- \
    --disable-asm \
    --disable-encoders --disable-hwaccels --disable-bsfs --disable-devices --disable-filters \
    --disable-protocols --enable-protocol=file \
    --disable-parsers --enable-parser=mpegaudio --enable-parser=h264 --enable-parser=hevc \
    --disable-demuxers --enable-demuxer=mov --enable-demuxer=mp3 --enable-demuxer=mpegts \
    --disable-decoders --enable-decoder=mp3 --enable-decoder=aac \
    --disable-muxers --enable-muxer=matroska \
    --enable-swresample

$PY chromium/scripts/build_ffmpeg.py android x64 --branding Chrome -- \
    --disable-asm \
    --disable-encoders --disable-hwaccels --disable-bsfs --disable-devices --disable-filters \
    --disable-protocols --enable-protocol=file \
    --disable-parsers --enable-parser=mpegaudio --enable-parser=h264 --enable-parser=hevc \
    --disable-demuxers --enable-demuxer=mov --enable-demuxer=mp3 --enable-demuxer=mpegts \
    --disable-decoders --enable-decoder=mp3 --enable-decoder=aac \
    --disable-muxers --enable-muxer=matroska \
    --enable-swresample

./chromium/scripts/copy_config.sh

echo "*****************************************************************"
echo "*****************************************************************"
echo "*****************************************************************"
echo "run ./chromium/scripts/generate_gn.py on macOS with conda py2 env"
echo "*****************************************************************"
echo "*****************************************************************"
echo "*****************************************************************"
#exit

./chromium/scripts/generate_gn.py

popd

#exit

#gn gen out/android_release_arm --args='target_os="android" target_cpu="arm" is_debug=false ffmpeg_branding="Chrome"'
#ninja -C out/android_release_arm libjingle_peerconnection_so
#cp out/android_release_arm/libjingle_peerconnection_so.so ../../webrtc_ios/src/sdk/android_gradle/webrtc/prebuilt_libs/armeabi-v7a/

#gn gen out/android_release_x86 --args='target_os="android" target_cpu="x86" is_debug=false ffmpeg_branding="Chrome"'
#ninja -C out/android_release_x86 libjingle_peerconnection_so
#cp out/android_release_x86/libjingle_peerconnection_so.so ../../webrtc_ios/src/sdk/android_gradle/webrtc/prebuilt_libs/x86/


#gn gen out/android_release_arm64 --args='target_os="android" target_cpu="arm64" is_debug=false ffmpeg_branding="Chrome"'
#ninja -C out/android_release_arm64 libjingle_peerconnection_so
#ninja -C out/android_release_arm64 libwebrtc
#cp out/android_release_arm64/libjingle_peerconnection_so.so ../../webrtc_ios/src/sdk/android_gradle/webrtc/prebuilt_libs/arm64-v8a/


echo "start"
gn gen out/android_release_arm64 --args='target_os="android" target_cpu="arm64" is_debug=true ffmpeg_branding="Chrome"'
ninja -C out/android_release_arm64 libjingle_peerconnection_so
ninja -C out/android_release_arm64 libwebrtc


#gn gen out/android_release_x64 --args='target_os="android" target_cpu="x64" is_debug=false ffmpeg_branding="Chrome"'
#ninja -C out/android_release_x64 libjingle_peerconnection_so
#cp out/android_release_x64/libjingle_peerconnection_so.so ../../webrtc_ios/src/sdk/android_gradle/webrtc/prebuilt_libs/x86_64/
