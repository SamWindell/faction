@echo off
set CommonCompilerFlags=-Od -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -DFACTION_WINDOWS=1 -FC -Z7 /EHsc
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib Opengl32.lib /SUBSYSTEM:WINDOWS

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

cl %CommonCompilerFlags% ..\code\faction.cpp ..\code\faction_win32.cpp /link %CommonLinkerFlags%
popd
