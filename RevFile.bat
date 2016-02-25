@echo off
if not [%1]==[]  cd %1
set REVFILE_H=.\src\Revfile.h
set REVFILE_TXT=.\RevFile.txt
set REVFILE_TMPL=.\src\RevFile.tmpl
if exist %REVFILE_H% del %REVFILE_H%

if exist "%PROGRAMFILES%\TortoiseSVN\bin\SubWCRev.exe"        "%PROGRAMFILES%\TortoiseSVN\bin\SubWCRev.exe"  .  %REVFILE_TMPL%  %REVFILE_H%
if exist "%PROGRAMW6432%\TortoiseSVN\bin\SubWCRev.exe"        "%PROGRAMW6432%\TortoiseSVN\bin\SubWCRev.exe"  .  %REVFILE_TMPL%  %REVFILE_H%

if exist "%PROGRAMFILES%\git\bin\git.exe"        set GIT_EXE="%PROGRAMFILES%\git\bin\git.exe"
if exist "%PROGRAMW6432%\Git\bin\git.exe"        set GIT_EXE="%PROGRAMW6432%\Git\bin\git.exe"

if exist %REVFILE_H% goto :lab1

if [%GIT_EXE%]==[]  goto  :end

echo #ifndef _REVFILE_H  > %REVFILE_H%
echo #define _REVFILE_H  >> %REVFILE_H%
rem 
rem #define REVISION_NUM      $WCREV$
rem #define REVISION          "Rev.$WCREV$ "
rem #define MODIFIED          "Loc.version $WCMODS?Modified:Not modified$ "
rem #define DATE              "Build $WCDATE$ "
rem #define REV_RANGE         "Rev range $WCRANGE$ "
rem #define MIXED_REV         "$WCMIXED?Mixed revision WC:No mixed revision WC$ "
rem #define URL_SVN           "URL $WCURL$ "

for /F "delims=.- tokens=2" %%i in ('%GIT_EXE% describe --tags') DO @set REVISION_NUM=%%i
echo #define REVISION_NUM   %REVISION_NUM%  >> %REVFILE_H%
for /F " tokens=*" %%i in ('%GIT_EXE% describe --tags') DO @set REVISION=%%i
echo #define REVISION    "%REVISION% " >> %REVFILE_H%
echo #define MODIFIED    ""            >> %REVFILE_H%

%GIT_EXE% show -s --format^="%%ci %%h %%H" > git_out.tmp
for /F "usebackq tokens=1,2,4,5" %%i in (git_out.tmp) DO @set BUILD=%%i %%j & set shorthash=%%k & set HASH=%%l

echo #define DATE       "Build %BUILD%"      >> %REVFILE_H%
echo #define REV_RANGE  "hash %shorthash% "  >> %REVFILE_H%
echo #define MIXED_REV  "HASH %HASH% "       >> %REVFILE_H%
del git_out.tmp

for /F "tokens=2" %%i in ('%GIT_EXE% remote -v') DO @set GIT_URL=%%i
echo #define URL_SVN    "URL %GIT_URL% "     >> %REVFILE_H%

echo #endif              >> %REVFILE_H%

:lab1
rem Строки ниже проверяют что мы в Git репозитории. Если нет, то сообщение ошибки выведется в REVFILE_TXT
%GIT_EXE% describe --tags   2>%REVFILE_TXT%
rem если фавйл REVFILE_TXT не пуст, то была ошибка.
for /F "tokens=1" %%i in (%REVFILE_TXT%) DO @set _GIT_ERR_=%%i
if [%_GIT_ERR_%]==[]  goto :git_rep
goto :end
:git_rep

echo Repositories: > %REVFILE_TXT%
for /F "tokens=2" %%i in ('%GIT_EXE% remote -v') DO   echo [%%i] >> %REVFILE_TXT%
echo Version     >> %REVFILE_TXT%
%GIT_EXE% describe --tags >> %REVFILE_TXT%
echo ^| Date        ^| Time      ^| hash     ^| HASH ^| >> %REVFILE_TXT%
%GIT_EXE% show -s --format^="%%ci %%h %%H" > git_out.tmp
for /F "usebackq tokens=1,2,4,5" %%i in (git_out.tmp) DO @set BUILD_DATE=%%i & set BUILD_TIME=%%j & set shorthash=%%k & set HASH=%%l
echo ^| %BUILD_DATE% ^| %BUILD_TIME% ^| %shorthash% ^| %HASH% ^| >> %REVFILE_TXT%
del git_out.tmp

echo Branches: >> %REVFILE_TXT%
%GIT_EXE% branch --contains  >> %REVFILE_TXT%
%GIT_EXE% log -n1 --format="Message:%%n %%B" >> %REVFILE_TXT% 

:end
