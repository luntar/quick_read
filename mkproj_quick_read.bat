set BN=quick_read
set BLDDIR=bld%BN%
set PNAME=%BN%.pro
@echo %PNAME% into %BLDDIR%
:: VERIFY CORRECT PATH
if not exist %PNAME% goto ErrorWrongDir
:: VERIFY PROJECT BUILD DIR IS DOES NOT EXIST
if exist %BLDDIR% goto MustDeleteBldDirFirst

:: CREATE PROJECT DIRECTOR
mkdir %BLDDIR% 
pushd  %BLDDIR%

:: RUN QMAKE TO CREATE VS2015.32bit PROJECT
qmake -platform win32-msvc -tp vc -r ..\%PNAME% 
goto DONE


:ErrorWrongDir
@echo Call this script in the root of the subdir projects
goto DONE

:MustDeleteBldDirFirst
@echo Must delete old build dir %BLDDIR%
goto DONE


:DONE
popd
goto :EOF
