set EnginePath=%1
set BuildPath=%2

if [%BuildPath%] == [] set BuildPath=%CD%

%EnginePath%/Engine/Build/BatchFiles/RunUAT.bat BuildPlugin -Plugin=%CD%/Plugins/Gunsmith/Gunsmith.uplugin -Package=%BuildPath%/Gunsmith -TargetPlatforms=Win64+Android+Linux -Rocket -Verbose
PAUSE