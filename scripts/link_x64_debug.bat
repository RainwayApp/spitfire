set "folder=link"
rmdir /s/q %folder%\Libx64\Debug

For /f %%b In ('Dir webrtc-checkout\src\out\x64\Debug\ /b /s /a:d') Do (
       Robocopy %%b %folder%\Libx64\Debug\ *.lib /xx /np
    )
pause
