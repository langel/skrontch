@echo off
setlocal enabledelayedexpansion

set BUILD_MODE=dev
set UNITY_MODE=1
set ARG_ERROR=
for %%a in (%*) do (
	if /I "%%~a"=="dev" (
		set BUILD_MODE=dev
	) else (
		if /I "%%~a"=="release" (
			set BUILD_MODE=release
		) else (
			if /I "%%~a"=="unity" (
				set UNITY_MODE=1
			) else (
				if /I "%%~a"=="nounity" (
					set UNITY_MODE=0
				) else (
					set ARG_ERROR=%%~a
				)
			)
		)
	)
)

if not "!ARG_ERROR!"=="" (
	echo Unknown argument: !ARG_ERROR!
	echo Usage: build.bat [dev^|release] [unity^|nounity]
	exit /b 1
)

if "%SDL2_DIR%"=="" (
	echo SDL2_DIR is not set.
	echo Example: set SDL2_DIR=C:\Users\knoxb\source\msys\mingw64
	exit /b 1
)

set SRC_DIR=src
set BUILD_DIR=build
set INCLUDE_DIR=include
set OUTPUT_DIR=%BUILD_DIR%
if /I "%BUILD_MODE%"=="release" set OUTPUT_DIR=%BUILD_DIR%\release

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

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
set COPY_SDL_DLL=1

if /I "%BUILD_MODE%"=="release" (
	set COPY_SDL_DLL=0
	if not exist "%SDL_LIB%\libSDL2.a" (
		echo Release mode requires static SDL2 library: %SDL_LIB%\libSDL2.a
		echo Install/update your MinGW SDL2 package with static libs.
		exit /b 1
	)
	if not exist "%SDL_LIB%\libSDL2main.a" (
		echo Release mode requires static SDL2main library: %SDL_LIB%\libSDL2main.a
		echo Install/update your MinGW SDL2 package with static libs.
		exit /b 1
	)
	set SDL_STATIC_SYS_LIBS=-lm -lkernel32 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lversion -luuid -ladvapi32 -lsetupapi -lshell32 -ldinput8
	set LDFLAGS=-L"%SDL_LIB%" -lmingw32 -Wl,-Bstatic -lSDL2main -lSDL2 -Wl,-Bdynamic !SDL_STATIC_SYS_LIBS! -mwindows
)

if "%UNITY_MODE%"=="1" (
	echo [%BUILD_MODE%] Unity build enabled.
	set SOURCES="%SRC_DIR%\unity_build.c"
	if not exist "%SRC_DIR%\unity_build.c" (
		echo Unity source file not found: %SRC_DIR%\unity_build.c
		exit /b 1
	)
) else (
	echo [%BUILD_MODE%] Gathering source files...
	set SOURCES=
	for /r "%SRC_DIR%" %%f in (*.c) do (
		if /I not "%%~nxf"=="unity_build.c" set SOURCES=!SOURCES! "%%f"
	)
)

if "%SOURCES%"=="" (
	echo No C source files found under %SRC_DIR%.
	exit /b 1
)

set RESOURCE_OBJ=
if exist "skrontch.rc" (
	echo [%BUILD_MODE%] Compiling Windows resources...
	set RESOURCE_OBJ=%OUTPUT_DIR%\skrontch.res
	windres "skrontch.rc" -O coff -o "!RESOURCE_OBJ!"
	if errorlevel 1 exit /b 1
)

set EXTRA_OBJECTS=
if not "%RESOURCE_OBJ%"=="" set EXTRA_OBJECTS="%RESOURCE_OBJ%"

echo [%BUILD_MODE%] Compiling and linking executable...
gcc %CFLAGS% %SOURCES% %EXTRA_OBJECTS% -o "%OUTPUT_DIR%\skrontch.exe" %LDFLAGS%
if errorlevel 1 exit /b 1

if "%COPY_SDL_DLL%"=="1" (
	echo [%BUILD_MODE%] Resolving SDL runtime DLL...
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

	if not "!SDL_RUNTIME_DLL!"=="" (
		echo [%BUILD_MODE%] Copying SDL runtime DLL to output...
		copy /Y "!SDL_RUNTIME_DLL!" "%OUTPUT_DIR%\" > nul
	)
)

if /I "%BUILD_MODE%"=="release" (
	if exist "assets" (
		echo [%BUILD_MODE%] Copying GUI assets...
		xcopy "assets" "%OUTPUT_DIR%\assets\" /E /I /Y > nul
	) else (
		echo Warning: assets directory not found; release may miss GUI assets.
	)
)

echo Build complete [%BUILD_MODE%]: %OUTPUT_DIR%\skrontch.exe
