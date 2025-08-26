@echo off
setlocal

echo ==========================================================
echo           BUILDING THIRD-PARTY DEPENDENCIES
echo ==========================================================

REM -- Configuration --
REM Define the generator name WITHOUT quotes
set CMAKE_GENERATOR=Visual Studio 17 2022
set CMAKE_BUILD_TYPE=Release
set SCRIPT_DIR=%~dp0

REM -- Paths --
set LUAU_SOURCE_DIR=%SCRIPT_DIR%luau
set LUAU_BUILD_DIR=%LUAU_SOURCE_DIR%\build
set VENDOR_DIR=%SCRIPT_DIR%third_party\vendor

REM Assuming Raylib is already present in VENDOR_DIR\raylib

echo.
echo [1/3] Configuring Luau...
echo      Source: %LUAU_SOURCE_DIR%
echo      Build:  %LUAU_BUILD_DIR%
echo      Install Destination: %VENDOR_DIR%\luau

if not exist "%LUAU_BUILD_DIR%" mkdir "%LUAU_BUILD_DIR%"
pushd "%LUAU_BUILD_DIR%"

REM Generate the build files and set the install location
REM *** CHANGE HERE: Add quotes around %CMAKE_GENERATOR% when it's used ***
cmake "%LUAU_SOURCE_DIR%" -G "%CMAKE_GENERATOR%" -A x64 -DCMAKE_INSTALL_PREFIX="%VENDOR_DIR%/luau"
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] CMake configuration for Luau failed.
    popd
    exit /b 1
)

echo.
echo [2/3] Building Luau (%CMAKE_BUILD_TYPE%)...
cmake --build . --config %CMAKE_BUILD_TYPE%
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Luau build failed.
    popd
    exit /b 1
)

echo.
echo [3/3] Installing Luau to vendor directory...
cmake --install . --config %CMAKE_BUILD_TYPE%
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Luau installation failed.
    popd
    exit /b 1
)

popd
echo.
echo ==========================================================
echo          DEPENDENCIES BUILT AND INSTALLED!
echo          You can now run build_engine.bat
echo ==========================================================