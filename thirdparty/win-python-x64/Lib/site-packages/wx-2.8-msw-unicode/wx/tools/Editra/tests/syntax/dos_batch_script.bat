:: Syntax Highlighting Test File for Dos Batch Scripts
:: Comment Line

:BEGIN
set HELLO="WORLD"
set WORLD="HELLO"
set HELLOWORLD="%WORLD% %HELLO%"

if "%1" == "/?" goto HELP
if "%1" == "go" goto EXECUTE

:HELP
echo .
echo Here is test file to show some dos batch highlighting
echo .
goto END

:EXECUTE
echo .
echo %HELLOWORLD%
echo .

:END
