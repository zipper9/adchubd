@echo off
set OPENSSL_DIR=openssl-native.3.0.15.1
set OPENSSL_ARCH=%OPENSSL_DIR%.nupkg
set VC_VER=v142
set OPENSSL_URL=https://www.nuget.org/api/v2/package/openssl-native/3.0.15.1
set PATH=%PATH%;"C:\Program Files\7-Zip"

call :checkprog curl

call :download %OPENSSL_ARCH% %OPENSSL_URL%
rmdir /S /Q %OPENSSL_DIR%
7z x -y -o%OPENSSL_DIR% %OPENSSL_ARCH%
if errorlevel 1 (
 echo Failed to extract %OPENSSL_ARCH%
 exit 1
)

rmdir /S /Q include
rmdir /S /Q lib
move %OPENSSL_DIR%\include .
mkdir lib
mkdir lib\Win32
mkdir lib\x64
copy %OPENSSL_DIR%\lib\win-x86\native\libcrypto_static_%VC_VER%.lib  lib\Win32\libcrypto.lib
copy %OPENSSL_DIR%\lib\win-x86\native\libssl_static_%VC_VER%.lib     lib\Win32\libssl.lib
copy %OPENSSL_DIR%\lib\win-x64\native\libcrypto_static_%VC_VER%.lib  lib\x64\libcrypto.lib
copy %OPENSSL_DIR%\lib\win-x64\native\libssl_static_%VC_VER%.lib     lib\x64\libssl.lib
rmdir /S /Q %OPENSSL_DIR%
del /Q %OPENSSL_ARCH%

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
