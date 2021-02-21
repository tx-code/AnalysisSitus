@echo off

echo "*** Running jenkins-custom.bat..."

set "JENKINS_CMAKE_BIN=C:\Program Files\CMake\bin"

REM ===========================================================================
REM  Configuration of Network Resources
REM ===========================================================================

set "JENKINS_3RDPARTIES_ARCHIVE_DIR=D:\Work\AnalysisSitus-libpacks\win\msvc2019_x64"
set "JENKINS_3RDPARTIES_ARCHIVE=analysissitus-libpack-msvc2019.zip"
set "JENKINS_LAST_BUILD_DIR=D:\Work\!situ"
