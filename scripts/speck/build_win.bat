@echo off
setlocal

set SCRIPTDIR=%~dp0
set PROJDIR=%SCRIPTDIR%..\..\
set MSBUILDBIN="C:\Program Files (x86)\MSBuild\14.0\Bin\msbuild.exe"

cd %PROJDIR%

@echo on
set BUILDDIR=%PROJDIR%build_win32\
set OUTPUTDIR=%PROJDIR%output\win32\
rmdir /S/Q %BUILDDIR%
mkdir %BUILDDIR%
cd %BUILDDIR%
cmake %PROJDIR% -G"Visual Studio 14 2015" -DBUILD_SHARED_LIBS=ON -DCMAKE_CONFIGURATION_TYPES=Release
%MSBUILDBIN% ALL_BUILD.vcxproj /property:Configuration=Release
cd ..

set BUILDDIR=%PROJDIR%build_win64\
set OUTPUTDIR=%PROJDIR%output\win64\
rmdir /S/Q %BUILDDIR%
mkdir %BUILDDIR%
cd %BUILDDIR%
cmake %PROJDIR% -G"Visual Studio 14 2015 Win64" -DBUILD_SHARED_LIBS=ON -DENABLE_AVX2=ON -DCMAKE_CONFIGURATION_TYPES=Release
%MSBUILDBIN% ALL_BUILD.vcxproj /property:Configuration=Release
cd ..
