 @echo off
 setlocal enabledelayedexpansion
 
 if "%SDL2_DIR%"=="" (
     echo SDL2_DIR is not set.
     echo Example: set SDL2_DIR=C:\libs\SDL2
     exit /b 1
 )
 
 set SRC_DIR=src
 set BUILD_DIR=build
 set INCLUDE_DIR=include
 
 if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
 
set SDL_INCLUDE=
if exist "%SDL2_DIR%\include\SDL2\SDL.h" (
    set SDL_INCLUDE=%SDL2_DIR%\include\SDL2
) else if exist "%SDL2_DIR%\include\SDL.h" (
    set SDL_INCLUDE=%SDL2_DIR%\include
) else if exist "%SDL2_DIR%\SDL.h" (
    set SDL_INCLUDE=%SDL2_DIR%
)

if "%SDL_INCLUDE%"=="" (
    echo Could not find SDL headers under SDL2_DIR: %SDL2_DIR%
    echo Expected one of:
    echo   %SDL2_DIR%\include\SDL2\SDL.h
    echo   %SDL2_DIR%\include\SDL.h
    echo   %SDL2_DIR%\SDL.h
    exit /b 1
)

set SDL_LIB=%SDL2_DIR%\lib\x64
 
 if not exist "%SDL_LIB%" (
     set SDL_LIB=%SDL2_DIR%\lib
 )
 
set CFLAGS=-std=c99 -Wall -Wextra -I"%INCLUDE_DIR%" -I"%SRC_DIR%" -I"%SDL_INCLUDE%"
set LDFLAGS=-L"%SDL_LIB%" -lmingw32 -lSDL2main -lSDL2 -mwindows
 
 set SOURCES=
 for /r "%SRC_DIR%" %%f in (*.c) do (
     set SOURCES=!SOURCES! "%%f"
 )
 
 if "%SOURCES%"=="" (
     echo No C source files found under %SRC_DIR%.
     exit /b 1
 )
 
if exist "skrontch.rc" (
    windres "skrontch.rc" -O coff -o "%BUILD_DIR%\skrontch.res"
)

gcc %CFLAGS% %SOURCES% "%BUILD_DIR%\skrontch.res" -o "%BUILD_DIR%\skrontch.exe" %LDFLAGS%
 if errorlevel 1 exit /b 1
 
set SDL_RUNTIME_DLL=
if exist "%SDL2_DIR%\bin\SDL2.dll" (
    set SDL_RUNTIME_DLL=%SDL2_DIR%\bin\SDL2.dll
) else if exist "%SDL2_DIR%\bin\libSDL2-2.0.dll" (
    set SDL_RUNTIME_DLL=%SDL2_DIR%\bin\libSDL2-2.0.dll
) else if exist "%SDL_LIB%\SDL2.dll" (
    set SDL_RUNTIME_DLL=%SDL_LIB%\SDL2.dll
) else if exist "%SDL_LIB%\libSDL2-2.0.dll" (
    set SDL_RUNTIME_DLL=%SDL_LIB%\libSDL2-2.0.dll
)

if not "%SDL_RUNTIME_DLL%"=="" (
    copy /Y "%SDL_RUNTIME_DLL%" "%BUILD_DIR%\" > nul
)
 
 echo Build complete: %BUILD_DIR%\skrontch.exe
