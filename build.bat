@echo off

IF NOT EXIST build mkdir build
pushd build


where /q cl && (
    call cl -arch:AVX2 -Od -Zi -W4 -nologo -wd4505 -wd4189 ..\main.cpp -Fe:main.exe -D__PROFILER=1
    call cl -arch:AVX2 -Od -Zi -W4 -nologo -wd4505 -wd4189 ..\haversine_generator.cpp -Fe:haversine_generator.exe
)

popd
