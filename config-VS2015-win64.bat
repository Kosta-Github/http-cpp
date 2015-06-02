@echo off
cmake -H. -BBuild -G "Visual Studio 14 2015" -A x64 -DBUILD_SHARED_LIBS=true
