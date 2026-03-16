@setlocal
@set FREEMARKERCLI=%FISLIBRARY%\freemarker-cli-2.0.0-BETA-5\bin\freemarker-cli.bat

@echo Creating View Template
@call %FREEMARKERCLI% ^
-DTYPE=SHELL ^
-DTARGETOS=%TARGETOS% ^
-t %FISDEVELOPMENT%\%P4SHELL%\p4.view.ftl %1 ^
-o %2

@endlocal
