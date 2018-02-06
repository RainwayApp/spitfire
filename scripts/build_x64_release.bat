@ECHO OFF
cd webrtc-checkout\src\
call gn gen out/x64/Release --args="is_debug=false target_cpu=\"x64\" symbol_level=0 rtc_enable_sctp=true fatal_linker_warnings=false treat_warnings_as_errors=false rtc_include_tests=true"
call ninja -C out/x64/Release
pause
