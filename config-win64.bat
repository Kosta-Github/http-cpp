@echo off
cmake -H. -BBuild -G "Visual Studio 11 Win64" -DBUILD_SHARED_LIBS=true
