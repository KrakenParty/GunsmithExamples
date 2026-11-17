<#
.SYNOPSIS
    Validates the plugin archive produced by PackagePlugin.bat by installing it into a fresh, empty
    project, compiling it, launching the editor and checking the startup log for errors and warnings.

.DESCRIPTION
    1. Locates <OutputRoot>\<BuildID>\<EngineVersion>.zip (the PackagePlugin.bat output).
    2. Generates a minimal empty C++ project in WorkRoot. A C++ project is needed because the
       archive only contains source, so the plugin has to be compiled before the editor can load it.
    3. Extracts the archive into the project's Plugins\<PluginName> directory.
    4. Builds the project's editor target (compile errors fail the step).
    5. Launches the editor unattended, which quits as soon as startup finishes.
    6. Scans the editor log for Error/Warning lines, minus anything matched by the ignore file,
       and reports them to TeamCity via service messages.

    Exits non-zero if the step should fail.
#>
param(
    [Parameter(Mandatory = $true)][string]$EngineVersion,
    [Parameter(Mandatory = $true)][string]$OutputRoot,
    [Parameter(Mandatory = $true)][string]$BuildID,
    [Parameter(Mandatory = $true)][string]$EnginePath,

    [string]$PluginName = "Gunsmith",

    # Where the throwaway project is generated. Wiped at the start of every run.
    [string]$WorkRoot = "",

    # File of regexes (one per line, '#' for comments). Matching log lines are not reported.
    [string]$IgnoreFile = "",

    # Report startup warnings as TeamCity warnings instead of failing the step (errors always fail).
    [switch]$AllowWarnings,

    # Fail if the plugin produces C++ compiler warnings (otherwise they are only reported).
    [switch]$FailOnCompileWarnings,

    # Launch the editor with a real RHI. Off by default because build agents running as a
    # service usually have no GPU/desktop. Turn on to catch shader/material errors.
    [switch]$WithRendering,

    [int]$EditorTimeoutMinutes = 30
)

$ErrorActionPreference = "Stop"

$ProjectName = "PluginValidation"

if (-not $WorkRoot) { $WorkRoot = Join-Path $env:TEMP "$($PluginName)PluginValidation\$EngineVersion" }
if (-not $IgnoreFile) { $IgnoreFile = Join-Path $PSScriptRoot "ValidatePluginIgnore.txt" }

$ArchivePath  = Join-Path $OutputRoot "$BuildID\$EngineVersion.zip"
$ProjectDir   = Join-Path $WorkRoot $ProjectName
$ProjectFile  = Join-Path $ProjectDir "$ProjectName.uproject"
$PluginDir    = Join-Path $ProjectDir "Plugins\$PluginName"
$LogDir       = Join-Path $WorkRoot "Logs"
$BuildLog     = Join-Path $LogDir "Build.log"
$EditorLog    = Join-Path $LogDir "EditorStartup.log"
$ArtifactPath = "PluginValidation/$EngineVersion"

# ---------------------------------------------------------------------------------------------
# TeamCity helpers
# ---------------------------------------------------------------------------------------------

function Escape-TeamCity([string]$Value) {
    $Value = $Value -replace '\|', '||'
    $Value = $Value -replace "'", "|'"
    $Value = $Value -replace '\[', '|['
    $Value = $Value -replace '\]', '|]'
    $Value = $Value -replace "`r", '|r'
    $Value = $Value -replace "`n", '|n'
    return $Value
}

function Write-TeamCity([string]$Message) {
    Write-Host "##teamcity[$Message]"
}

function Report-Problem([string]$Description) {
    # Descriptions over 4000 chars are truncated by TeamCity; keep them short and unique.
    if ($Description.Length -gt 3500) { $Description = $Description.Substring(0, 3500) + "..." }
    Write-TeamCity "buildProblem description='$(Escape-TeamCity $Description)'"
}

function Report-Warning([string]$Text) {
    Write-TeamCity "message text='$(Escape-TeamCity $Text)' status='WARNING'"
}

function Publish-Logs {
    if (Test-Path $LogDir) {
        Write-TeamCity "publishArtifacts '$(Escape-TeamCity "$LogDir => $ArtifactPath")'"
    }
}

function Fail([string]$Description) {
    Report-Problem $Description
    Publish-Logs
    exit 1
}

# ---------------------------------------------------------------------------------------------
# 1. Locate the packaged plugin
# ---------------------------------------------------------------------------------------------

Write-TeamCity "blockOpened name='Prepare validation project'"

if (-not (Test-Path $ArchivePath)) { Fail "Packaged plugin not found: $ArchivePath" }

$EditorExe = Join-Path $EnginePath "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$BuildBat  = Join-Path $EnginePath "Engine\Build\BatchFiles\Build.bat"
if (-not (Test-Path $EditorExe)) { Fail "Editor not found: $EditorExe" }
if (-not (Test-Path $BuildBat))  { Fail "Build.bat not found: $BuildBat" }

# ---------------------------------------------------------------------------------------------
# 2. Generate an empty C++ project
# ---------------------------------------------------------------------------------------------

if (Test-Path $WorkRoot) { Remove-Item $WorkRoot -Recurse -Force }
New-Item -ItemType Directory -Force -Path $ProjectDir, $PluginDir, $LogDir | Out-Null

$SourceDir = Join-Path $ProjectDir "Source"
$ModuleDir = Join-Path $SourceDir $ProjectName
New-Item -ItemType Directory -Force -Path $ModuleDir, (Join-Path $ProjectDir "Config") | Out-Null

$Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
function Write-ProjectFile([string]$Path, [string]$Content) {
    [System.IO.File]::WriteAllText($Path, $Content, $Utf8NoBom)
}

Write-ProjectFile $ProjectFile @"
{
	"FileVersion": 3,
	"EngineAssociation": "",
	"Category": "",
	"Description": "Throwaway project used by CI to validate the packaged $PluginName plugin",
	"Modules": [
		{
			"Name": "$ProjectName",
			"Type": "Runtime",
			"LoadingPhase": "Default",
			"AdditionalDependencies": [
				"Engine",
				"CoreUObject",
				"$PluginName"
			]
		}
	],
	"Plugins": [
		{
			"Name": "$PluginName",
			"Enabled": true
		}
	]
}
"@

Write-ProjectFile (Join-Path $SourceDir "$($ProjectName)Editor.Target.cs") @"
using UnrealBuildTool;

public class $($ProjectName)EditorTarget : TargetRules
{
	public $($ProjectName)EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("$ProjectName");
	}
}
"@

Write-ProjectFile (Join-Path $SourceDir "$ProjectName.Target.cs") @"
using UnrealBuildTool;

public class $($ProjectName)Target : TargetRules
{
	public $($ProjectName)Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("$ProjectName");
	}
}
"@

Write-ProjectFile (Join-Path $ModuleDir "$ProjectName.Build.cs") @"
using UnrealBuildTool;

public class $ProjectName : ModuleRules
{
	public $ProjectName(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "AssetRegistry", "$PluginName" });
	}
}
"@

# The project module loads every asset in the plugin's content folder once the engine has
# initialised, so load errors (missing references, Blueprint compile errors, etc.) end up in the
# startup log. Driven by -ValidatePluginContent=/<MountPoint> on the command line.
Write-ProjectFile (Join-Path $ModuleDir "$ProjectName.cpp") @"
#include "Modules/ModuleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"

DEFINE_LOG_CATEGORY_STATIC(LogPluginValidation, Log, All);

class F$($ProjectName)Module : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FCoreDelegates::OnPostEngineInit.AddRaw(this, &F$($ProjectName)Module::LoadPluginContent);
	}

	virtual void ShutdownModule() override
	{
		FCoreDelegates::OnPostEngineInit.RemoveAll(this);
	}

private:
	void LoadPluginContent()
	{
		FString ContentPath;
		if (!FParse::Value(FCommandLine::Get(), TEXT("ValidatePluginContent="), ContentPath))
		{
			return;
		}

		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		AssetRegistry.ScanPathsSynchronous({ ContentPath }, true);

		TArray<FAssetData> Assets;
		AssetRegistry.GetAssetsByPath(FName(*ContentPath), Assets, true);

		if (Assets.Num() == 0)
		{
			UE_LOG(LogPluginValidation, Error, TEXT("No assets found under %s"), *ContentPath);
			return;
		}

		UE_LOG(LogPluginValidation, Display, TEXT("Loading %d assets under %s"), Assets.Num(), *ContentPath);

		int32 Failed = 0;
		for (const FAssetData& Asset : Assets)
		{
			UE_LOG(LogPluginValidation, Display, TEXT("Loading %s"), *Asset.GetObjectPathString());
			if (!Asset.GetAsset())
			{
				UE_LOG(LogPluginValidation, Error, TEXT("Failed to load %s"), *Asset.GetObjectPathString());
				++Failed;
			}
		}

		UE_LOG(LogPluginValidation, Display, TEXT("Loaded %d/%d assets under %s"), Assets.Num() - Failed, Assets.Num(), *ContentPath);
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(F$($ProjectName)Module, $ProjectName, "$ProjectName");
"@

Write-ProjectFile (Join-Path $ProjectDir "Config\DefaultEngine.ini") @"
[/Script/EngineSettings.GameMapsSettings]
EditorStartupMap=/Engine/Maps/Entry
GameDefaultMap=/Engine/Maps/Entry
"@

# ---------------------------------------------------------------------------------------------
# 3. Install the packaged plugin into the project
# ---------------------------------------------------------------------------------------------

Write-Host "Extracting $ArchivePath -> $PluginDir"
if (Get-Command 7z -ErrorAction SilentlyContinue) {
    & 7z x "$ArchivePath" "-o$PluginDir" -y | Out-Null
    if ($LASTEXITCODE -ne 0) { Fail "Failed to extract $ArchivePath (7z exit code $LASTEXITCODE)" }
} else {
    Expand-Archive -Path $ArchivePath -DestinationPath $PluginDir -Force
}

if (-not (Test-Path (Join-Path $PluginDir "$PluginName.uplugin"))) {
    Fail "$PluginName.uplugin not found at the root of $ArchivePath"
}

Write-TeamCity "blockClosed name='Prepare validation project'"

# ---------------------------------------------------------------------------------------------
# 4. Compile
# ---------------------------------------------------------------------------------------------

Write-TeamCity "blockOpened name='Compile $PluginName in empty project'"

$BuildArgs = "$($ProjectName)Editor Win64 Development -Project=`"$ProjectFile`" -WaitMutex -NoHotReloadFromIDE"
& cmd.exe /c "`"$BuildBat`" $BuildArgs 2>&1" | Tee-Object -FilePath $BuildLog | Write-Host
$BuildExitCode = $LASTEXITCODE

$CompileWarnings = @(Select-String -Path $BuildLog -Pattern ':\s+warning\s+[A-Z]*\d+\s*:|:\s+warning:' |
                     Where-Object { $_.Line -match [regex]::Escape($PluginName) } |
                     ForEach-Object { $_.Line.Trim() } | Select-Object -Unique)

foreach ($Warning in $CompileWarnings) { Report-Warning "Compile warning: $Warning" }

Write-TeamCity "blockClosed name='Compile $PluginName in empty project'"

if ($BuildExitCode -ne 0) { Fail "Compiling $PluginName in an empty project failed (exit code $BuildExitCode). See $ArtifactPath/Build.log" }
if ($FailOnCompileWarnings -and $CompileWarnings.Count -gt 0) {
    Fail "$($CompileWarnings.Count) compile warning(s) in $PluginName"
}

# ---------------------------------------------------------------------------------------------
# 5. Launch the editor and quit once startup completes
# ---------------------------------------------------------------------------------------------

Write-TeamCity "blockOpened name='Editor startup'"

$EditorArgs = @(
    "`"$ProjectFile`"",
    "-unattended", "-nopause", "-nosplash", "-nosound",
    "-NoLiveCoding",
    "-stdout", "-FullStdOutLogOutput",
    "-abslog=`"$EditorLog`"",
    "-ValidatePluginContent=/$PluginName",
    "-ExecCmds=`"QUIT_EDITOR`""
)
if (-not $WithRendering) { $EditorArgs += "-nullrhi" }

Write-Host "$EditorExe $($EditorArgs -join ' ')"
$Editor = Start-Process -FilePath $EditorExe -ArgumentList $EditorArgs -NoNewWindow -PassThru
$null = $Editor.Handle  # Cache the handle so ExitCode is available after exit

$TimedOut = -not $Editor.WaitForExit($EditorTimeoutMinutes * 60 * 1000)
if ($TimedOut) {
    Write-Host "Editor did not exit within $EditorTimeoutMinutes minutes, killing it"
    try { & taskkill.exe /PID $Editor.Id /T /F | Out-Null } catch {}
}
$EditorExitCode = $Editor.ExitCode

Write-TeamCity "blockClosed name='Editor startup'"

# ---------------------------------------------------------------------------------------------
# 6. Analyse the log
# ---------------------------------------------------------------------------------------------

if (-not (Test-Path $EditorLog)) { Fail "Editor did not produce a log at $EditorLog (exit code $EditorExitCode)" }

$IgnorePatterns = @()
if (Test-Path $IgnoreFile) {
    $IgnorePatterns = @(Get-Content $IgnoreFile | ForEach-Object { $_.Trim() } |
                        Where-Object { $_ -and -not $_.StartsWith("#") })
    Write-Host "Loaded $($IgnorePatterns.Count) ignore pattern(s) from $IgnoreFile"
}

function Test-Ignored([string]$Line) {
    foreach ($Pattern in $IgnorePatterns) { if ($Line -match $Pattern) { return $true } }
    return $false
}

# UE log lines look like "[2026.11.17-10.00.00:000][  0]LogCategory: Warning: message"
$LogLineRegex = '^(?:\[[^\]]*\]\[\s*\d+\])?(?<Category>[A-Za-z0-9_]+): (?<Severity>Error|Warning): (?<Message>.*)$'
$FatalRegex   = 'Fatal error|=== Critical error ===|Unhandled Exception|Assertion failed'

$Errors   = New-Object System.Collections.Generic.List[string]
$Warnings = New-Object System.Collections.Generic.List[string]
$Ignored  = 0

foreach ($Line in [System.IO.File]::ReadLines($EditorLog)) {
    $Match = [regex]::Match($Line, $LogLineRegex)
    $IsFatal = $Line -match $FatalRegex
    if (-not $Match.Success -and -not $IsFatal) { continue }

    $Entry = if ($Match.Success) { "$($Match.Groups['Category'].Value): $($Match.Groups['Severity'].Value): $($Match.Groups['Message'].Value)" } else { $Line.Trim() }
    if (Test-Ignored $Entry) { $Ignored++; continue }

    if ($IsFatal -or $Match.Groups['Severity'].Value -eq "Error") { $Errors.Add($Entry) } else { $Warnings.Add($Entry) }
}

Write-Host ""
Write-Host "Editor exit code: $EditorExitCode"
Write-Host "Startup errors:   $($Errors.Count)"
Write-Host "Startup warnings: $($Warnings.Count)"
Write-Host "Ignored lines:    $Ignored"

foreach ($Entry in $Errors)   { Write-Host "ERROR:   $Entry" }
foreach ($Entry in $Warnings) { Write-Host "WARNING: $Entry" }

# Make the counts visible as TeamCity statistics so they can be charted / used in failure conditions
Write-TeamCity "buildStatisticValue key='PluginValidation.$EngineVersion.Errors' value='$($Errors.Count)'"
Write-TeamCity "buildStatisticValue key='PluginValidation.$EngineVersion.Warnings' value='$($Warnings.Count)'"

$Failed = $false

if ($TimedOut) {
    Report-Problem "[$EngineVersion] Editor did not finish starting up within $EditorTimeoutMinutes minutes"
    $Failed = $true
} elseif ($EditorExitCode -ne 0) {
    Report-Problem "[$EngineVersion] Editor exited with code $EditorExitCode"
    $Failed = $true
}

# One build problem per distinct line so TeamCity can track new vs. existing problems
foreach ($Entry in ($Errors | Select-Object -Unique)) {
    Report-Problem "[$EngineVersion] $Entry"
    $Failed = $true
}

foreach ($Entry in ($Warnings | Select-Object -Unique)) {
    if (-not $AllowWarnings) {
        Report-Problem "[$EngineVersion] $Entry"
        $Failed = $true
    } else {
        Report-Warning "[$EngineVersion] $Entry"
    }
}

Publish-Logs

if ($Failed) { exit 1 }

Write-Host "$PluginName $EngineVersion started cleanly in an empty project"
exit 0
