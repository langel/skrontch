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
 
 set SDL_INCLUDE=%SDL2_DIR%\
 set SDL_LIB=%SDL2_DIR%\lib\x64
 
 if not exist "%SDL_LIB%" (
     set SDL_LIB=%SDL2_DIR%\lib
 )
 
set CFLAGS=-std=c99 -Wall -Wextra -I"%INCLUDE_DIR%" -I"%SDL_INCLUDE%"
 set LDFLAGS=-L"%SDL_LIB%" -lmingw32 -lSDL2main -lSDL2 -lSDL_image
 
 set SOURCES=
 for /r "%SRC_DIR%" %%f in (*.c) do (
     set SOURCES=!SOURCES! "%%f"
 )
 
 if "%SOURCES%"=="" (
     echo No C source files found under %SRC_DIR%.
     exit /b 1
 )
 
 gcc %CFLAGS% %SOURCES% -o "%BUILD_DIR%\skrontch.exe" %LDFLAGS%
 if errorlevel 1 exit /b 1
 
 if exist "%SDL_LIB%\SDL2.dll" (
     copy /Y "%SDL_LIB%\SDL2.dll" "%BUILD_DIR%\SDL2.dll" > nul
 )
if exist "%SDL_LIB%\SDL2_image.dll" (
    copy /Y "%SDL_LIB%\SDL2_image.dll" "%BUILD_DIR%\SDL2_image.dll" > nul
)
 
 echo Build complete: %BUILD_DIR%\skrontch.exe
