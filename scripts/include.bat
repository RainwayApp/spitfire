@ECHO OFF
set "folder=include"
rmdir /s/q %folder%
mkdir %folder%

xcopy /s webrtc-checkout\src\*.h %folder%\webrtc\

rmdir %folder%\webrtc\third_party


xcopy /s webrtc-checkout\src\third_party\libvpx\*.h %folder%\third_party\libvpx\


xcopy /s webrtc-checkout\src\third_party\libyuv\include\*.h %folder%\third_party\libyuv\include\


xcopy /s webrtc-checkout\src\third_party\jsoncpp\*.h %folder%\third_party\jsoncpp\



xcopy /s webrtc-checkout\src\third_party\boringssl\*.h %folder%\third_party\boringssl\

