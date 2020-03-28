echo off

cd ..\

echo ------------------------------------------------------------------
echo Building ...

if not defined DevEnvDir (
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_x64
)

echo Building 64 bit binaries...
msbuild guitard.sln /p:configuration=release /p:platform=x64 /nologo /verbosity:minimal /fileLogger /m /flp:logfile=build-win.log;errorsonly;append

echo ------------------------------------------------------------------
echo Making Installer ...

"%ProgramFiles(x86)%\Inno Setup 6\iscc" /Q ".\installer\GuitarD.iss"
