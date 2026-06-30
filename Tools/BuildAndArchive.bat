set EnginePath=%1
set OutputRoot=%2
%EnginePath%/Engine/Build/BatchFiles/RunUAT.bat BuildGraph -Script=%CD%/Tools/BuildGraph/BuildAndStage.xml -Target="Build All Platforms" -set:WorkspaceRoot=%CD% -set:OutputRoot=%OutputRoot%