gn gen out/android_debug_arm64 --args='target_os="android" target_cpu="arm64" is_debug=true ffmpeg_branding="Chrome"'
ninja -C out/android_debug_arm64 libjingle_peerconnection_so
ninja -C out/android_debug_arm64 libwebrtc
