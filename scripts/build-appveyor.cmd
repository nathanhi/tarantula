@ECHO OFF

SETLOCAL ENABLEEXTENSIONS

:: Check for crucial variables before beginning
IF NOT DEFINED BUILDTYPE ECHO ERROR: BUILDTYPE variable not set. Can either be "MINGW" or "VS". && EXIT /B 1
IF /I "%BUILDTYPE%" NEQ "MINGW" (
	IF /I "%BUILDTYPE%" NEQ "VS" (
		ECHO ERROR: BUILDTYPE variable has invalid value "%BUILDTYPE%". Either "MINGW" or "VS" are a valid build-type.
		EXIT /B 1
	)
)

:: Ignore PLATFORM and MSVS_TOOLSET with MINGW
IF /I "%BUILDTYPE%" == "VS" (
	IF NOT DEFINED PLATFORM ECHO ERROR: PLATFORM variable not set. Can either be "x86", "x64" or "ARM". && EXIT /B 1
	IF NOT DEFINED MSVS_TOOLSET ECHO ERROR: MSVS_TOOLSET variable not set. Can be any given MSVS version. && EXIT /B 1
	IF /I "%PLATFORM%" NEQ "x86" (
		IF /I "%PLATFORM%" NEQ "x64" (
			IF /I "%PLATFORM%" NEQ "ARM" (
				ECHO ERROR: PLATFORM variable has invalid value "%PLATFORM%". Either "x86", "x64" or "ARM" is allowed.
				EXIT /B 1
			)
		)
	)
	IF /I "%PLATFORM%" == "ARM" ECHO WARN: ARM platform can only be built if ARM Desktop SDK is available.
	GOTO MSVC_BUILD
)

IF /I "%BUILDTYPE%" == "MINGW" GOTO MINGW_BUILD

EXIT /B 1

:MSVC_BUILD
ECHO Starting MSVC build..

IF /I "%PLATFORM%" == "x64" CALL "C:\Program Files (x86)\Microsoft Visual Studio %MSVS_TOOLSET%.0\VC\vcvarsall.bat" amd64
IF /I "%PLATFORM%" == "x86" CALL "C:\Program Files (x86)\Microsoft Visual Studio %MSVS_TOOLSET%.0\VC\vcvarsall.bat" x86
IF /I "%PLATFORM%" == "ARM" CALL "C:\Program Files (x86)\Microsoft Visual Studio %MSVS_TOOLSET%.0\VC\vcvarsall.bat" amd64_arm

nmake /f Makefile.nm clean sample
EXIT /B %ERRORLEVEL%

:MINGW_BUILD
ECHO Starting MINGW build...
IF EXIST C:\MinGW\bin SET PATH=%PATH%;C:\MinGW\bin

mingw32-make clean sample
EXIT /B %ERRORLEVEL%

ENDLOCAL