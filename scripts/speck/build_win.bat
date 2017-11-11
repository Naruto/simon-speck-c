@echo off
setlocal

set SCRIPTDIR=%~dp0
set PROJDIR=%SCRIPTDIR%..\..\
set LIBSDIR=%PROJDIR%libs\
set ORIGPATH=%PATH%

cd %PROJDIR%

@echo on

rmdir /S/Q %LIBSDIR%windows
mkdir %LIBSDIR%
mkdir %LIBSDIR%windows

set BUILDDIR=%PROJDIR%build_win32\
set OUTPUTDIR=%LIBSDIR%windows\win32\
rmdir /S/Q %BUILDDIR%
mkdir %BUILDDIR%
cd %BUILDDIR%
cmake -G"Visual Studio 15 2017" -DENABLE_TESTING=ON -DBUILD_SHARED_LIBS=ON -DCMAKE_CONFIGURATION_TYPES=Release .. || exit /b 1
cmake --build . --target ALL_BUILD -- /p:Configuration=Release || exit /b 1
set PREVPATH=%PATH%
set PATH=%PROJDIR%build_win32/Release;%PATH%
ctest -C Release . || exit /b 1
set PATH=%PREVPATH%
mkdir %OUTPUTDIR%
copy %BUILDDIR%Release\speck.dll %OUTPUTDIR%
cd ..

set BUILDDIR=%PROJDIR%build_win64\
set OUTPUTDIR=%LIBSDIR%windows\win64\
rmdir /S/Q %BUILDDIR%
mkdir %BUILDDIR%
cd %BUILDDIR%
cmake -G"Visual Studio 15 2017 Win64" -DENABLE_TESTING=ON -DENABLE_AVX2=ON -DBUILD_SHARED_LIBS=ON -DCMAKE_CONFIGURATION_TYPES=Release .. || exit /b 1
cmake --build . --target ALL_BUILD -- /p:Configuration=Release || exit /b 1
set PREVPATH=%PATH%
set PATH=%PROJDIR%build_win64\Release;%PATH%
ctest -C Release . || exit /b 1
set PATH=%PREVPATH%
mkdir %OUTPUTDIR%
copy %BUILDDIR%Release\speck.dll %OUTPUTDIR%
cd ..
