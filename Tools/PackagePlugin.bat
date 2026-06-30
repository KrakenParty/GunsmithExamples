set EngineVersion=%1
set Password=%~2
set OutputRoot=%~3
set BuildID=%4
set EnginePath=%~5
set PluginPath=%CD%\GunsmithExamples\Plugins\Gunsmith
set ArchiveRoot=%OutputRoot%\%BuildID%
set ArchivePath=%ArchiveRoot%\%EngineVersion%.zip

powershell.exe -executionpolicy bypass -file %CD%\Tools\EditVersionJson.ps1 %PluginPath%\Gunsmith.uplugin %EngineVersion%
powershell.exe -executionpolicy bypass -file %CD%\Tools\EditVersionJson.ps1 %CD%\GunsmithExamples\GunsmithExamples.uproject %EngineVersion% EngineAssociation

:: Resave all assets with the new engine version
cmd /C ""%EnginePath%\Engine\Build\BatchFiles\RunUAT.bat" BuildGraph -Script=%CD%\Tools\BuildGraph\BuildForResave.xml -Target="Build" -set:WorkspaceRoot=%CD%\GunsmithExamples" /S
"%EnginePath%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" %CD%\GunsmithExamples\GunsmithExamples.uproject -run=ResavePackages -OnlyUnversioned -AutoCheckOutPackages -IgnoreChangeList -KeepPackageGUIDOnSave -PackageSubstring=/Gunsmith/Content/

if not exist "%ArchiveRoot%" mkdir "%ArchiveRoot%"

del "%ArchivePath%"
7z a "%ArchivePath%" %PluginPath%\Config %PluginPath%\Content %PluginPath%\Source %PluginPath%\Gunsmith.uplugin -p%Password%

git restore %PluginPath%\Gunsmith.uplugin
git restore %CD%\GunsmithExamples\GunsmithExamples.uproject
