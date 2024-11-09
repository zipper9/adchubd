@echo off
set BOOST_DIR=boost_1_86_0
set BOOST_ARCH=%BOOST_DIR%.7z
set BOOST_URL=https://archives.boost.io/release/1.86.0/source/%BOOST_ARCH%
set PATH=%PATH%;"C:\Program Files\7-Zip"

rem call :checkprog 7z
call :checkprog curl

call :download %BOOST_ARCH% %BOOST_URL%
rmdir /S /Q %BOOST_DIR%
7z x -y -x!%BOOST_DIR%\doc\* -x!%BOOST_DIR%\more\* -x!%BOOST_DIR%\tools\* -x!*.html %BOOST_ARCH%
if errorlevel 1 (
 echo Failed to extract %BOOST_ARCH%
 exit 1
)

move %BOOST_DIR%\boost .
move %BOOST_DIR%\libs .
rmdir /S /Q %BOOST_DIR%
del /Q %BOOST_ARCH%

echo Done
exit

:checkprog
where /Q %1
if errorlevel 1 (
 echo A required program is missing: %1.exe
 exit 1
)
exit /B

:download
echo Downloading %1
curl -L -o %1 %2
if errorlevel 1 (
 echo Failed to download %1
 exit 1
)
exit /B
