for %%i in ("%~dp0..") do set "folder=%%~fi"
echo %folder%

xcopy /s %folder%\webrtc-checkout\src\webrtc\*.h %folder%\includes\webrtc\
pause

xcopy /s %folder%\webrtc-checkout\src\third_party\libvpx\*.h %folder%\includes\third_party\libvpx\
pause

xcopy /s %folder%\webrtc-checkout\src\third_party\libyuv\include\*.h %folder%\includes\third_party\libyuv\include\
pause

xcopy /s %folder%\webrtc-checkout\src\third_party\jsoncpp\*.h %folder%\includes\third_party\jsoncpp\

pause

xcopy /s %folder%\webrtc-checkout\src\third_party\boringssl\*.h %folder%\includes\third_party\boringssl\

pause