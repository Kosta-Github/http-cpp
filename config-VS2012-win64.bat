@echo off
cmake -H. -BBuild -G "Visual Studio 11 2012" -A x64 -DBUILD_SHARED_LIBS=true
