REM ********** Initialize env for msvc 2015 cl compiler **********
SET PATH=C:\Windows;C:\Windows\system32
REM Set up \Microsoft Visual Studio 2015, where <arch> is amd64, x86, etc.
CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

REM ********** Update include & lib to support xp win sdk 7.1A **********
SET PATH=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin;%PATH%
SET INCLUDE=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include;%INCLUDE%
SET LIB=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib;%LIB%
SET CL=/D_USING_V140_SDK71_;%CL%

REM ********** 3rd party build tools binaries: ruby, perl, python **********
SET PATH=C:\Strawberry\perl\bin;%PATH%

REM ********** Set up qt source env **********
SET _ROOT=C:\Qt\5.6.2
SET PATH=%_ROOT%\qtbase\bin;%_ROOT%\gnuwin32\bin;%PATH%
REM Uncomment the below line when using a git checkout of the source repository
REM SET PATH=%_ROOT%\qtrepotools\bin;%PATH%
SET _ROOT=
SET QMAKESPEC=win32-msvc2015

REM Generate makefile
REM C:\Qt\5.6.2\qtbase\configure.bat -confirm-license -opensource -platform win32-msvc2015 -debug-and-release -prefix E:\QT5.9.1_Static\VS2015 -plugin-sql-sqlite -plugin-sql-odbc -qt-zlib -qt-libpng -qt-libjpeg -opengl desktop -qt-freetype -no-qml-debug -no-angle -nomake tests -nomake examples -skip qtquickcontrols -skip qtquickcontrols2 -skip qtsensors -skip qtdoc -mp

C:\Qt\5.6.2\qtbase\configure.bat -target xp -confirm-license -opensource -platform win32-msvc2015 -debug -opengl desktop -mp -no-dbus -qt-libpng -qt-libjpeg
