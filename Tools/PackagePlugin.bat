set EngineVersion=%1
set OutputRoot=%~2
set BuildID=%3
set EnginePath=%~4
set PluginPath=%CD%\Plugins\Gunsmith
set ArchiveRoot=%OutputRoot%\%BuildID%
set ArchivePath=%ArchiveRoot%\%EngineVersion%.zip

powershell.exe -executionpolicy bypass -file %CD%\Tools\EditVersionJson.ps1 %PluginPath%\Gunsmith.uplugin %EngineVersion%
powershell.exe -executionpolicy bypass -file %CD%\Tools\EditVersionJson.ps1 %CD%\GunsmithExamples.uproject %EngineVersion% EngineAssociation

:: Resave all assets with the new engine version
cmd /C ""%EnginePath%\Engine\Build\BatchFiles\RunUAT.bat" BuildGraph -Script=%CD%\Tools\BuildGraph\BuildForResave.xml -Target="Build" -set:WorkspaceRoot=%CD%" /S
"%EnginePath%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" %CD%\GunsmithExamples.uproject -run=ResavePackages -AutoCheckOutPackages -PackageSubstring=/Gunsmith/Content/

if not exist "%ArchiveRoot%" mkdir "%ArchiveRoot%"

del "%ArchivePath%"
7z a "%ArchivePath%" %PluginPath%\Config %PluginPath%\Content %PluginPath%\Source %PluginPath%\Gunsmith.uplugin

git reset --hard
cmd Plugins/Gunsmith
git reset --hard
