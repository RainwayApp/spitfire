@ECHO OFF
set "folder=link"
rmdir /s/q %folder%\Libx86\Debug

For /f %%b In ('Dir webrtc-checkout\src\out\x86\Debug\ /b /s /a:d') Do (
       Robocopy %%b %folder%\Libx86\Debug\ *.lib /xx /np
    )
pause
