Powershell.exe -ExecutionPolicy Bypass -File .\GenerateLibFromDll.ps1 "Libs\x64\zlib1.dll" "x64"
Powershell.exe -ExecutionPolicy Bypass -File .\GenerateLibFromDll.ps1 "Libs\Win32\zlib1.dll" "x86"