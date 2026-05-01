param()

$root = Split-Path -Parent $PSScriptRoot
$projectsDir = Join-Path $root "projects"
$bash = "C:\msys64\usr\bin\bash.exe"

if (-not (Test-Path -LiteralPath $bash)) {
  throw "MSYS2 bash not found at $bash. Install MSYS2 with MinGW32 make/gcc first."
}

$resolvedProjects = (Resolve-Path -LiteralPath $projectsDir).Path
if ($resolvedProjects -match '^([A-Za-z]):\\(.*)$') {
  $drive = $matches[1].ToLower()
  $rest = $matches[2] -replace '\\', '/'
  $msysProjects = "/$drive/$rest"
} else {
  throw "Cannot convert project path for MSYS2: $resolvedProjects"
}

& $bash -lc "cd '$msysProjects' && PATH=/usr/bin:/mingw32/bin make PLATFORM=RGNANO_SIM -j2"
exit $LASTEXITCODE
