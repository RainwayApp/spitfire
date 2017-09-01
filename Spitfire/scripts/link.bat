for %%i in ("%~dp0..") do set "folder=%%~fi"
echo %folder%

 For /f %%b In ('Dir %folder%\webrtc-checkout\src\out\release-x64\ /b /s /a:d') Do (
       Robocopy %%b %folder%\lib_x64\ *.lib /xx /np
    )
	
cd %folder%\lib_x64\

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat" 


del %folder%\lib_x64\protobuf_full.lib >nul 2>&1
del %folder%\lib_x64\webrtc-all.lib >nul 2>&1

lib /out:%folder%\lib_x64\webrtc-all.lib %folder%\lib_x64\*.lib

for %%i in (*.*) do if not "%%i"=="webrtc-all.lib" del /q "%%i"

pause
