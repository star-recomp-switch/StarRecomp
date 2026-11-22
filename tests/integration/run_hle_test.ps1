$env:PATH = "C:\msys64\ucrt64\bin;C:\devkitPro\msys2\usr\bin;" + $env:PATH
& "D:\Claude Stuff\KirbyRecomp\build\bin\kirbyrecomp_hle_tests.exe"
Write-Host "Exit code: $LASTEXITCODE"
