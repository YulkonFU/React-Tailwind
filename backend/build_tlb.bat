@echo off
chcp 65001

echo Setting up Visual Studio environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

echo Compiling IDL to TLB...
set MIDL_PATH="C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\midl.exe"

REM 直接在当前目录生成文件
%MIDL_PATH% /nologo /tlb .\DeviceHandler.tlb /h .\DeviceHandler_i.h .\DeviceHandler.idl

if %ERRORLEVEL% EQU 0 (
    echo Successfully generated type library.
    echo Files generated in current directory:
    echo - DeviceHandler.tlb
    echo - DeviceHandler_i.h
) else (
    echo Failed to generate type library.
)

echo Done.
pause