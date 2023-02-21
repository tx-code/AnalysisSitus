@echo off

echo "*** Running jenkins-configure-and-build-sdk.bat..."

REM ===========================================================================
REM Configure environment on Jenkins machine
REM ===========================================================================

call "%~dp0"jenkins-custom.bat

REM ===========================================================================
REM Copy and unpack 3-rd parties
REM ===========================================================================

mkdir 3rd-parties-arc

echo "*** Copying the 3-rd parties %JENKINS_3RDPARTIES_ARCHIVE_DIR%\%JENKINS_3RDPARTIES_ARCHIVE%"

copy %JENKINS_3RDPARTIES_ARCHIVE_DIR%\%JENKINS_3RDPARTIES_ARCHIVE% 3rd-parties-arc
copy %JENKINS_3RDPARTIES_ARCHIVE_DIR%\%JENKINS_3RDPARTIES_FBX_ARCHIVE% 3rd-parties-arc
copy %JENKINS_3RDPARTIES_ARCHIVE_DIR%\7z.dll 3rd-parties-arc
copy %JENKINS_3RDPARTIES_ARCHIVE_DIR%\7z.exe 3rd-parties-arc
copy jenkins-install-products.bat 3rd-parties-arc

cd 3rd-parties-arc

call "jenkins-install-products.bat" ..\3rd-parties

cd ..

set "JENKINS_3RDPARTIES=%~dp03rd-parties"

REM ===========================================================================
REM Configure environment
REM ===========================================================================

set "JENKINS_JOB_DIR=%~dp0"
echo JENKINS_JOB_DIR: %JENKINS_JOB_DIR%

REM ===========================================================================
REM Prepare build and install directories for CMake
REM ===========================================================================

echo Create cmake-build-dir and cmake-install-dir

mkdir cmake-build-dir
mkdir cmake-install-dir

REM ===========================================================================
REM Run CMake from build directory: configure
REM ===========================================================================

cd cmake-build-dir

echo "*** Running CMake configuration..."

"%JENKINS_CMAKE_BIN%\cmake.exe" -G"Visual Studio 16 2019" -A x64 ^
-DUSE_THREADING=false -DUSE_RAPIDJSON=true -DUSE_FBX_SDK=false -DUSE_MOBIUS=true  -DUSE_NETGEN=false ^
-D3RDPARTY_mobius_DIR=%JENKINS_3RDPARTIES_MOBIUS_DIR% ^
-DDISTRIBUTION_TYPE=Algo -D3RDPARTY_DIR:PATH=%JENKINS_3RDPARTIES% -DINSTALL_DIR:PATH=../cmake-install-dir ../

REM ===========================================================================
REM CMake build and install
REM ===========================================================================

echo "*** Running compilation in Release mode..."

"%JENKINS_CMAKE_BIN%\cmake.exe" --build . --config Release --target INSTALL
