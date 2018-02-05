@ECHO OFF
rmdir /s/q webrtc-checkout
mkdir webrtc-checkout
cd webrtc-checkout
call fetch --no-history webrtc
cd src
call git branch -r
call git checkout branch-heads/65
call gclient sync
Echo 'Please edit the checked out branch to build with the MD flag'
pause
