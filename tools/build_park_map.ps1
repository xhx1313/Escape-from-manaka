param(
    [string]$GameDir = "D:\Study\SecretFlasherManaka",
    [string]$OutputDirectory = ""
)

$ErrorActionPreference = "Stop"
$root = Split-Path $PSScriptRoot -Parent
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $root "assets\maps"
}

$gameData = Join-Path $GameDir "SecretFlasherManaka_Data\data.unity3d"
if (-not (Test-Path -LiteralPath $gameData)) {
    throw "Required game data not found: $gameData"
}

& python (Join-Path $PSScriptRoot "render_multifloor_atlas.py") `
    --game-data $gameData `
    --output-directory $OutputDirectory `
    --scene park `
    --size 2560 `
    --preview-size 512
if ($LASTEXITCODE -ne 0) {
    throw "Unified park atlas rendering failed."
}

Write-Host "Park map completed: $(Join-Path $OutputDirectory 'park')"
