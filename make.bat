@ECHO off
SETLOCAL
REM Makefile for Windows, 32 or 64 bit version
REM Use "make 32", "make 64" or "make clean"

REM Get paths of Visual studio and the resource compiler
call make-rc-path.bat
call make-vs-path.bat

REM Choose 32 or 64 bit version
IF NOT EXIST out mkdir out
IF NOT EXIST out\windows32 mkdir out\windows32
IF NOT EXIST out\windows64 mkdir out\windows64
IF "%1"=="32" (
   GOTO BUILD32
)
IF "%1"=="64" (
   GOTO BUILD64
)
IF /i "%1"=="clean" (
   GOTO CLEAN
)
GOTO NOBUILD

:CLEAN
    ECHO Remove build output files..
    DEL /Q out\windows32\*
    DEL /Q out\windows64\*
    ECHO Cleaned up.
    GOTO DONE

:BUILD32
    SET RC32=%WRC%\x86
    SET CC32=%VSVC%\bin
    SET Path=%CC32%;%RC32%;%Path%
    SET OUT=out\windows32
    GOTO BUILD

:BUILD64
    SET RC64=%WRC%\x64
    SET CC64=%VSVC%\bin\amd64
    SET Path=%CC64%;%RC64%;%Path%
    SET OUT=out\windows64
    GOTO BUILD

:BUILD
    ECHO Building %1 bit version..
    REM Load compiler environment
    CALL vcvars%1.bat
    REM Compile resource file
    CALL rc /nologo /fo %OUT%\resource.res src\resource\resource%1.rc
    REM Source files
    SET objn[0]=main_app
    SET objf[0]=main_app.cpp
    SET objn[1]=main
    SET objf[1]=win32\main.cpp
    SET objn[2]=config
    SET objf[2]=config.cpp
    SET objn[3]=db
    SET objf[3]=db.cpp
    SET objn[4]=tabtree
    SET objf[4]=tabtree\tabtree.cpp
    SET objn[5]=tabtreeparser
    SET objf[5]=tabtree\tabtreeparser.cpp
    SET objn[6]=rijndael
    SET objf[6]=crypto\rijndael.c
    SET objn[7]=aes_ctr_256
    SET objf[7]=crypto\aes_ctr_256.c
    SET objn[8]=hmac_sha3_256
    SET objf[8]=crypto\hmac_sha3_256.c
    SET objn[9]=pbkdf
    SET objf[9]=crypto\pbkdf.c
    SET objn[10]=utfconvert
    SET objf[10]=win32\utfconvert.cpp
    SET objn[11]=create_db
    SET objf[11]=win32\create_db.cpp
    SET objn[12]=decrypt_db
    SET objf[12]=win32\decrypt_db.cpp
    SET objn[13]=editor
    SET objf[13]=win32\editor.cpp
    SET objn[14]=message
    SET objf[14]=win32\message.cpp
    REM Compile source files to object files
    SET OBJFILES=
    SET /A i = 0
    :OBJLOOP
    IF DEFINED objn[%i%] (
        IF NOT %ERRORLEVEL%==0 GOTO COMPILATIONERR REM The error code of the last iteration is set after GOTO
        CALL cl /nologo /EHsc /UTF-8 /Os /c /Fo%OUT%\%%objn[%i%]%%.obj src\%%objf[%i%]%%
        SET OBJFILES=%OBJFILES% %OUT%\%%objn[%i%]%%.obj
        SET /a i = %i% + 1
        GOTO :OBJLOOP
    )
    IF NOT %ERRORLEVEL%==0 GOTO COMPILATIONERR
    REM Assemble comiled main with resource
    SET WIN32LIBS=gdi32.lib user32.lib comctl32.lib ole32.lib shell32.lib comdlg32.lib advapi32.lib
    CALL cl /nologo /EHsc /UTF-8 /O2 /Fe%OUT%\nanopass.exe %OUT%\resource.res %OBJFILES% %WIN32LIBS%
    ECHO Build finished.
    GOTO DONE

:COMPILATIONERR
    ECHO Compilation failed, exiting
    GOTO DONE

:NOBUILD
    ECHO Please use 'make 32', 'make 64' or 'make clean'
    PAUSE

:DONE
    ENDLOCAL
