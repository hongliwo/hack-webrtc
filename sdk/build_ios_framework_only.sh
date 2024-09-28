#!/bin/bash

set -e

py=$(python -c 'import sys; print(".".join(map(str, sys.version_info[0:1])))')
if [[ "$py" != "2" ]]; then
  echo "Please use py2 env"
  exit
fi


#gn gen out/ios_release_arm64 --args='target_os="ios" target_cpu="arm64" rtc_enable_symbol_export=true ios_enable_code_signing=false is_component_build=false use_goma=false rtc_libvpx_build_vp9=false ffmpeg_branding="Chrome" is_debug=false enable_dsyms=true enable_stripping = true rtc_include_tests=false ios_deployment_target="12.0" extra_cflags_objcc=["-DABSL_HAVE_THREAD_LOCAL"]'
#ninja -C out/ios_release_arm64 framework_objc
gn gen out/ios_debug_arm64 --args='target_os="ios" target_cpu="arm64" rtc_enable_symbol_export=true ios_enable_code_signing=false is_component_build=false use_goma=false rtc_libvpx_build_vp9=false ffmpeg_branding="Chrome" is_debug=true enable_dsyms=true enable_stripping = true rtc_include_tests=false ios_deployment_target="12.0" '
ninja -C out/ios_debug_arm64 framework_objc
