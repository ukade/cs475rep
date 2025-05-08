@echo off
setlocal enabledelayedexpansion

:: Compile the C++ program (xiela_prjct2.cpp) with OpenMP support
cl /nologo /openmp xiela_prjct2.cpp /Fe:run.exe > nul

:: Run the compiled program (run2.exe) and redirect both stdout and stderr to output2.csv
run.exe >> output2.csv 2>&1

endlocal