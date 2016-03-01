@echo このスクリプトを実行するには、cmakeおよびmsbuildの双方がPATHに入っている必要があります。
@echo 通常、cmakeのパスは、C:\Program Files (x86)\CMake 2.8\bin。
@echo msbuildのパスは、C:\Windows\Microsoft.NET\Framework\v4.0.30319です。

@timeout 5

@if NOT EXIST build-win32 mkdir build-win32
@cd build-win32
@cmake ../ -G "Visual Studio 10"
@msbuild KobukiRTC.sln /p:Configuration=Release /p:Platform=WIN32
@cd ../
@exit /b