@echo off

echo "*** Running jenkins-run-tests.bat..."

REM ===========================================================================
REM Configure environment on Jenkins machine
REM ===========================================================================

call "%~dp0"jenkins-custom.bat

set "JENKINS_3RDPARTIES=%~dp03rd-parties"

set "ASI_TEST_DUMPING=ASI_TEST_DUMPING"
set "ASI_TEST_DATA=@ASI_TEST_DATA@"
set "ASI_TEST_SCRIPTS=@ASI_TEST_SCRIPTS@"
set "ASI_TEST_DESCR=@ASI_TEST_DESCR@"

REM ===========================================================================
REM Run tests
REM ===========================================================================

cd cmake-install-dir
DRAWEXE.exe -c testgrid -overwrite -parallel 4 -outdir "results"

REM ===========================================================================
REM Copy test results to the network drive
REM ===========================================================================

rem The following lines are commented out as there seems to be no sense in
rem copying test result to file server: all them are accessible via http
rem in Jenkins workspace, e.g. http://ssv:8080/.../...

rem echo Test results will be available at "%JENKINS_TEST_RESULT_DIR%"
rem cd %JENKINS_JOB_DIR%\test\results
rem xcopy /s . %JENKINS_TEST_RESULT_DIR% > NUL
