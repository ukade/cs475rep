.\@echo off
setlocal enabledelayedexpansion

for %%t in (1 2 4 8 10 12 16 18 20) do (
    for %%s in (100 500 1000 5000 10000 50000 100000) do (
        cl /nologo /openmp /DNUMT=%%t /DNUMTRIALS=%%s xiela_prjct1.cpp /Fe:run.exe > nul
        run.exe >> output.csv 2>&1
    )
)

endlocal