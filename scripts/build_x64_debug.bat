cd webrtc-checkout\src\
call gn gen out/x64/Debug --args="is_debug=true target_cpu=\"x64\" symbol_level=2 enable_nacl=false rtc_enable_sctp=true fatal_linker_warnings=false treat_warnings_as_errors=false"
call ninja -C out/x64/Debug
pause
