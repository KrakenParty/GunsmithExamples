:: Validates the archive produced by PackagePlugin.bat by loading it in an empty project.
:: Takes the same arguments as PackagePlugin.bat: EngineVersion OutputRoot BuildID EnginePath
set EngineVersion=%1
set OutputRoot=%~2
set BuildID=%3
set EnginePath=%~4

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0ValidatePlugin.ps1" -EngineVersion "%EngineVersion%" -OutputRoot "%OutputRoot%" -BuildID "%BuildID%" -EnginePath "%EnginePath%"
exit /b %ERRORLEVEL%
