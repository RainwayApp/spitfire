set "folder=link"
rmdir /s/q %folder%\Libx86\Release

For /f %%b In ('Dir webrtc-checkout\src\out\x86\Release\ /b /s /a:d') Do (
       Robocopy %%b %folder%\Libx86\Release\ *.lib /xx /np
    )
pause
