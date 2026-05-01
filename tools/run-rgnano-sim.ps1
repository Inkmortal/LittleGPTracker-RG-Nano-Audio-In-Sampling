param(
  [string]$ExePath = ".\projects\lgpt-rgnano-sim.exe",
  [string]$Script = "",
  [switch]$Skin
)

$root = Split-Path -Parent $PSScriptRoot
$resourceDir = Join-Path $root "projects\resources\RGNANO_SIM"
$exeFullPath = Resolve-Path -LiteralPath $ExePath -ErrorAction SilentlyContinue

if (-not $exeFullPath) {
  throw "Simulator executable not found: $ExePath. Build it with: cd projects; make PLATFORM=RGNANO_SIM"
}

$exeFullPath = $exeFullPath.Path
$exeDir = Split-Path -Parent $exeFullPath
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

$dataDir = Join-Path $exeDir "rgnano-sim-data"
New-Item -ItemType Directory -Force -Path (Join-Path $dataDir "tracks") | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $dataDir "samples") | Out-Null

$args = @()
if ($Script) {
  $args += "-RGNANOSIM_SCRIPT=$Script"
}
if ($Skin -or -not $Script) {
  $args += "-RGNANOSIM_SKIN=YES"
}

if ($args.Count -gt 0) {
  & $exeFullPath @args
} else {
  & $exeFullPath
}

exit $LASTEXITCODE
