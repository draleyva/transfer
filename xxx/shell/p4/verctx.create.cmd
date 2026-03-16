@setlocal
@set FREEMARKERCLI=%FISLIBRARY%\freemarker-cli-2.0.0-BETA-5\bin\freemarker-cli.bat

@echo Creating verctx configuration

@call %FREEMARKERCLI% ^
-DTYPE=SHELL ^
-DTARGETOS=%TARGETOS% ^
-t %FISDEVELOPMENT%\%P4SHELL%\verctx.ftl %1 ^
-o %2

@endlocal
