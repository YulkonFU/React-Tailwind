@echo off
echo Compiling IDL to TLB...

:: 设置MIDL路径（根据你的Windows SDK版本可能需要调整）
set MIDL_PATH="C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\midl.exe"

:: 编译IDL
%MIDL_PATH% /nologo /tlb DeviceHandler.tlb /h DeviceHandler_i.h "Source Files\DeviceHandler.idl"

if %ERRORLEVEL% EQU 0 (
    echo Successfully generated type library.
    echo Moving files to appropriate locations...
    
    :: 移动生成的文件到对应目录
    move /y DeviceHandler.tlb bin\
    move /y DeviceHandler_i.h "Header Files\"
) else (
    echo Failed to generate type library.
)

echo Done.
pause