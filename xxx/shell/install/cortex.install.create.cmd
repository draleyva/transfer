@setlocal
@set FREEMARKERCLI=%FISLIBRARY%\freemarker-cli-2.0.0-BETA-5\bin\freemarker-cli.bat

@echo Creating Install Shell
@call %FREEMARKERCLI% ^
-DTYPE=SHELL ^
-t %FISDEVELOPMENT%\%INSTALLSHELL%\cortex.install.ftl %1 ^
-o %2

@endlocal
