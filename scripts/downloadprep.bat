@ECHO OFF
rmdir /s/q webrtc-checkout
mkdir webrtc-checkout
cd webrtc-checkout
call fetch webrtc
cd src
call git branch -r
call git checkout branch-heads/63
call gclient sync
cd ..
cd ..
xcopy /Y BUILD.gn webrtc-checkout\src\build\config\win\BUILD.gn
pause
