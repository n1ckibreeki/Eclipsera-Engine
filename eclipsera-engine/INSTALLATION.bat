@echo off
setlocal EnableExtensions

rem
if not defined VSCMD_ARG_TGT_ARCH (
  call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
)

echo ==========================================================
echo
echo	███████  ██████ ██      ██ ██████  ███████ ███████ ██████   █████  
echo	██      ██      ██      ██ ██   ██ ██      ██      ██   ██ ██   ██ 
echo	█████   ██      ██      ██ ██████  ███████ █████   ██████  ███████ 
echo	██      ██      ██      ██ ██           ██ ██      ██   ██ ██   ██ 
echo	███████  ██████ ███████ ██ ██      ███████ ███████ ██   ██ ██   ██                                                                                                                                      
echo       MODIFICATION OF LUNAR-ENGINE BY LUNAR-ENGINE DEVELOPMENT
echo       ECLIPSERA ENGINE DIDN'T AFFRIMATIVE WITH LUNAR-ENGINE THIS IS A MODIFICATION ENGINE ONLY!
echo       
echo ==========================================================

set "CMAKE_GENERATOR=Ninja"
set "CMAKE_BUILD_TYPE=Release"
set "SCRIPT_DIR=%~dp0"

rem
set "CMAKE_BUILD_PARALLEL_LEVEL=8"
set "NINJA_STATUS=[%%e] [%%f/%%t] "

set "PROJECT_ROOT=%SCRIPT_DIR%"
set "BUILD_DIR=%PROJECT_ROOT%build"
set "DIST_DIR=%PROJECT_ROOT%dist"
set "CMAKE_PREFIX_PATH=%PROJECT_ROOT%third_party\vendor"


set "NO_CLEAN=0"
if /i "%~1"=="-no-clean" set "NO_CLEAN=1"

echo.
if "%NO_CLEAN%"=="0" (
  echo [1/4] Cleaning build directory only...
  if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"
) else (
  echo [1/4] Skipping clean (because -no-clean was specified)
)

echo.
echo [2/4] Configuring Eclipsera Engine...
echo      Build Dir: %BUILD_DIR%
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
pushd "%BUILD_DIR%"

rem Configure with Ninja
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=%CMAKE_BUILD_TYPE% -DCMAKE_PREFIX_PATH="%CMAKE_PREFIX_PATH%"
if %errorlevel% neq 0 ( echo.[ERROR] CMake configuration failed. & popd & exit /b 1 )

echo.
echo [3/4] Building Librebox (%CMAKE_BUILD_TYPE%)...
cmake --build . --config %CMAKE_BUILD_TYPE%
if %errorlevel% neq 0 ( echo.[ERROR] Eclipsera build failed. & popd & exit /b 1 )

echo.
echo [4/4] Installing to 'dist'...
cmake --install . --prefix "%DIST_DIR%"
if %errorlevel% neq 0 ( echo.[ERROR] Installation failed. & popd & exit /b 1 )

popd
echo.
echo ==========================================================
echo                BUILD SUCCESSFUL
echo ==========================================================