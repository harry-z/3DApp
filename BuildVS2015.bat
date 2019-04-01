@echo off
mkdir buildVS2015
pushd buildVS2015
@echo on
cmake ../ -G "Visual Studio 14 2015 Win64" -DRENDER_API:STRING=%2
cmake --build . --config %1
@echo off
popd