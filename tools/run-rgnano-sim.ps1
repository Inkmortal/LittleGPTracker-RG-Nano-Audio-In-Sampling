param(
  [string]$ExePath = ".\projects\lgpt-rgnano-sim.exe",
  [string]$Script = "",
  [switch]$Skin,
  [switch]$SeedSampleFixture,
  [switch]$ResetLastProject,
  [string]$ArtifactsDir = ""
)

$root = Split-Path -Parent $PSScriptRoot
$resourceDir = Join-Path $root "projects\resources\RGNANO_SIM"
$exeFullPath = Resolve-Path -LiteralPath $ExePath -ErrorAction SilentlyContinue
$routeHelpers = Join-Path $PSScriptRoot "rgnano-sim-routes.ps1"
. $routeHelpers

if (-not $exeFullPath) {
  throw "Simulator executable not found: $ExePath. Build it with: cd projects; make PLATFORM=RGNANO_SIM"
}

$exeFullPath = $exeFullPath.Path
$exeDir = Split-Path -Parent $exeFullPath
$runStarted = Get-Date

if ($ArtifactsDir) {
  New-Item -ItemType Directory -Force -Path $ArtifactsDir | Out-Null
  Get-ChildItem -LiteralPath $ArtifactsDir -File | Remove-Item -Force
}

Copy-Item -LiteralPath (Join-Path $resourceDir "config.xml") -Destination (Join-Path $exeDir "config.xml") -Force
Copy-Item -LiteralPath (Join-Path $resourceDir "mapping.xml") -Destination (Join-Path $exeDir "mapping.xml") -Force

$sdlDll = Join-Path $root "projects\resources\W32\SDL.dll"
if (Test-Path -LiteralPath $sdlDll) {
  Copy-Item -LiteralPath $sdlDll -Destination (Join-Path $exeDir "SDL.dll") -Force
}

$mingwBin = "C:\msys64\mingw32\bin"
foreach ($runtimeDll in @("libgcc_s_dw2-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll")) {
  $runtimePath = Join-Path $mingwBin $runtimeDll
  if (Test-Path -LiteralPath $runtimePath) {
    Copy-Item -LiteralPath $runtimePath -Destination (Join-Path $exeDir $runtimeDll) -Force
  }
}

$dataDir = Join-Path (Get-Location) "rgnano-sim-data"
New-Item -ItemType Directory -Force -Path (Join-Path $dataDir "tracks") | Out-Null
$sampleDir = Join-Path $dataDir "samples"
New-Item -ItemType Directory -Force -Path $sampleDir | Out-Null

function Write-TestWav {
  param([string]$Path)
  $sampleRate = 8000
  $durationSeconds = 1
  $sampleCount = $sampleRate * $durationSeconds
  $dataSize = $sampleCount * 2
  $writer = [System.IO.BinaryWriter]::new([System.IO.File]::Open($Path, [System.IO.FileMode]::Create))
  try {
    $writer.Write([System.Text.Encoding]::ASCII.GetBytes("RIFF"))
    $writer.Write([int](36 + $dataSize))
    $writer.Write([System.Text.Encoding]::ASCII.GetBytes("WAVEfmt "))
    $writer.Write([int]16)
    $writer.Write([Int16]1)
    $writer.Write([Int16]1)
    $writer.Write([int]$sampleRate)
    $writer.Write([int]($sampleRate * 2))
    $writer.Write([Int16]2)
    $writer.Write([Int16]16)
    $writer.Write([System.Text.Encoding]::ASCII.GetBytes("data"))
    $writer.Write([int]$dataSize)
    for ($i = 0; $i -lt $sampleCount; $i++) {
      $sample = [Int16]([Math]::Sin((2 * [Math]::PI * 440 * $i) / $sampleRate) * 12000)
      $writer.Write($sample)
    }
  } finally {
    $writer.Dispose()
  }
}

if ($SeedSampleFixture) {
  Write-TestWav -Path (Join-Path $sampleDir "rgnano-test-tone.wav")
}

if ($ResetLastProject) {
  $lastProjectFile = Join-Path $exeDir "last_project"
  if (Test-Path -LiteralPath $lastProjectFile) {
    Remove-Item -LiteralPath $lastProjectFile -Force
  }
}

$args = @()
if ($Script) {
  $scriptToRun = $Script
  $scriptText = Get-Content -LiteralPath $Script -Raw
  if ($scriptText -match '(?m)^\s*route\s+') {
    $expandedScript = Join-Path $exeDir "rgnano-sim-expanded.rgsim"
    Expand-RGNanoSimScript -ScriptPath $Script -OutputPath $expandedScript
    if (-not (Test-Path -LiteralPath $expandedScript)) {
      throw "Failed to expand RG Nano simulator script: $Script"
    }
    $scriptToRun = $expandedScript
  }
  $args += "-RGNANOSIM_SCRIPT=$scriptToRun"
}
$args += "-RGNANOSIM_LOG=$(Join-Path $exeDir 'rgnano-sim.log')"
if ($Skin -or -not $Script) {
  $args += "-RGNANOSIM_SKIN=YES"
}

if ($args.Count -gt 0) {
  & $exeFullPath @args
} else {
  & $exeFullPath
}

$exitCode = $LASTEXITCODE

if ($ArtifactsDir) {
  $logPath = Join-Path $exeDir "rgnano-sim.log"
  if (Test-Path -LiteralPath $logPath) {
    Copy-Item -LiteralPath $logPath -Destination (Join-Path $ArtifactsDir "rgnano-sim.log") -Force
  }
  Get-ChildItem -Path (Get-Location) -Filter "*.bmp" -File | Where-Object { $_.LastWriteTime -ge $runStarted } | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination (Join-Path $ArtifactsDir $_.Name) -Force
  }
}

exit $exitCode
