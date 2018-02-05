@ECHO OFF
cd webrtc-checkout\src\
call gn gen out/x86/Debug --args="is_debug=true target_cpu=\"x86\" symbol_level=2 enable_nacl=false rtc_enable_sctp=true fatal_linker_warnings=false treat_warnings_as_errors=false rtc_include_tests=false rtc_build_examples=false"
call ninja -C out/x86/Debug
pause
