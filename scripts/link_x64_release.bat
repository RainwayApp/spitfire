@ECHO OFF
set "folder=link"
rmdir /s/q %folder%\Libx64\Release

For /f %%b In ('Dir webrtc-checkout\src\out\x64\Release\ /b /s /a:d') Do (
       Robocopy %%b %folder%\Libx64\Release\ *.lib /xx /np
    )
pause
