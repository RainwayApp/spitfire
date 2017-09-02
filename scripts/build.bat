for %%i in ("%~dp0..") do set "folder=%%~fi"
cd %folder%\webrtc-checkout\src\
call gn gen out/release-x64 --ide="vs2017" --args="is_debug=false target_cpu=\"x64\" use_rtti=false is_component_build=false symbol_level=0"
call ninja -C out/release-x64
pause