 @echo off
 setlocal
 
 set EXE=build\skrontch.exe
 
 if not exist "%EXE%" (
     echo Build first with build.bat
     exit /b 1
 )
 
 "%EXE%"
