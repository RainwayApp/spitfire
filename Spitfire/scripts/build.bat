for %%i in ("%~dp0..") do set "folder=%%~fi"
cd %folder%\webrtc-checkout\src\
call gn gen out/release-x64 --ide="vs2017" --args="is_debug=false target_cpu=\"x64\" use_rtti=false is_component_build=false symbol_level=0"
pause
REM cmd /k ninja -C out/release-x64 boringssl field_trial_default protobuf_full p2p pc webrtc_common
REM pause