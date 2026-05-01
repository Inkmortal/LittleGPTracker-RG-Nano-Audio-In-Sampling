param(
  [string]$Task = "dev",
  [string]$App = "",
  [switch]$NoSetup
)
$projectRoot = $PSScriptRoot
& "C:\Users\danhc\Documents\Projects\Dev-Environment-Standards\Invoke-AgentRun.ps1" -ProjectRoot $projectRoot -Task $Task -App $App -NoSetup:$NoSetup
