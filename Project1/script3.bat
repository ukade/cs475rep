.\@echo off
setlocal enabledelayedexpansion

for %%t in (1 2 4 6 8) do (
    for %%s in (2 3 4 5 10 15 20 30 40 50) do (
        cl /nologo /openmp /DNUMT=%%t /DNUMCAPITALS=%%s xiela_prjct3.cpp /Fe:run.exe > nul
        run.exe >> output.csv 2>&1
    )
)

endlocal

:: .\script3.bat