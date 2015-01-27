@echo off
cmake -H. -BBuild -G "Visual Studio 11 Win64" -DCMAKE_BUILD_SHARED=true
