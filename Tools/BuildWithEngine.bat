set EnginePath=%1
%EnginePath%/Engine/Build/BatchFiles/RunUAT.bat BuildGraph -Script=%CD%/Tools/BuildGraph/BuildAllPlatforms.xml -Target="Build All Platforms" -set:WorkspaceRoot=%CD%/GunsmithExamples