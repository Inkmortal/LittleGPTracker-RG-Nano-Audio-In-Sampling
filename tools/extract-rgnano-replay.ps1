param(
  [Parameter(Mandatory = $true)][string]$LogPath,
  [Parameter(Mandatory = $true)][string]$OutputPath
)

$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("# Extracted from RG Nano hardware log")

foreach ($line in Get-Content -LiteralPath $LogPath) {
  if ($line -match '^\[RGNANO_REPLAY\]\s+(.+)$') {
    $command = $Matches[1].Trim()
    if ($command) {
      $lines.Add($command)
    }
  }
}

if ($lines.Count -eq 1) {
  throw "No RGNANO_REPLAY lines found in $LogPath"
}

$lines.Add("quit")
Set-Content -LiteralPath $OutputPath -Value $lines -Encoding ASCII
Write-Host "Wrote $($lines.Count - 2) replay commands to $OutputPath"
