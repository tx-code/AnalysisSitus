@echo off

echo "*** Running jenkins-clone-data.bat..."

REM ===========================================================================
REM Configure environment on Jenkins machine
REM ===========================================================================

call "%~dp0"jenkins-custom.bat

REM ===========================================================================
REM Clone repo with data
REM ===========================================================================

mkdir confidential-data

git clone git@gitlab.com:ssv/analysissitus_data.git ./confidential-data

xcopy /s confidential-data\data data\cad
