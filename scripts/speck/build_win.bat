@echo off
setlocal

set SCRIPTDIR=%~dp0
set PROJDIR=%SCRIPTDIR%..\..\
set LIBSDIR=%PROJDIR%libs\
set MSBUILDBIN="C:\Program Files (x86)\MSBuild\14.0\Bin\msbuild.exe"
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
cmake %PROJDIR% -G"Visual Studio 14 2015" -DBUILD_SHARED_LIBS=ON -DCMAKE_CONFIGURATION_TYPES=Release
%MSBUILDBIN% ALL_BUILD.vcxproj /property:Configuration=Release
set PATH=%PROJDIR%build_win32/Release;%PATH%
mkdir %OUTPUTDIR%
copy %BUILDDIR%Release\speck.dll %OUTPUTDIR%
cd ..

set BUILDDIR=%PROJDIR%build_win64\
set OUTPUTDIR=%PROJDIR%output\win64\
rmdir /S/Q %BUILDDIR%
mkdir %BUILDDIR%
cd %BUILDDIR%
cmake %PROJDIR% -G"Visual Studio 14 2015 Win64" -DBUILD_SHARED_LIBS=ON -DCMAKE_CONFIGURATION_TYPES=Release
%MSBUILDBIN% ALL_BUILD.vcxproj /property:Configuration=Release
set PATH=%PROJDIR%build_win64\Release;%PATH%
mkdir %OUTPUTDIR%
copy %BUILDDIR%Release\speck.dll %OUTPUTDIR%
cd ..
