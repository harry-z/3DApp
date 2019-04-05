@echo off
mkdir %4
pushd %4
@echo on
cmake ../ -G %1 -DRENDER_API:STRING=%3
cmake --build . --config %2
@echo off
popd