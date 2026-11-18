[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [string]$GameDir,

    [switch]$Launch
)

$ErrorActionPreference = "Stop"
$root = Split-Path $PSScriptRoot -Parent
$artifacts = Join-Path $root "artifacts"
$nativeProxy = Join-Path $artifacts "winhttp.dll"
$runtimeArchive = Join-Path $artifacts "runtime-assets.zip"
$version = (Get-Content -LiteralPath (Join-Path $root "VERSION") -Raw).Trim()

if (-not (Test-Path -LiteralPath $nativeProxy)) {
    throw "Built winhttp.dll was not found: $nativeProxy"
}
if (-not (Test-Path -LiteralPath $runtimeArchive)) {
    throw "Built runtime-assets.zip was not found: $runtimeArchive"
}

$gameRoot = [IO.Path]::GetFullPath($GameDir)
$gameExe = Join-Path $gameRoot "SecretFlasherManaka.exe"
$gameAssembly = Join-Path $gameRoot "GameAssembly.dll"
$unityPlayer = Join-Path $gameRoot "UnityPlayer.dll"
if (-not (Test-Path -LiteralPath $gameExe) -or
    -not (Test-Path -LiteralPath $gameAssembly) -or
    -not (Test-Path -LiteralPath $unityPlayer)) {
    throw "The selected directory is not a SecretFlasherManaka game directory."
}

$running = @(Get-Process -Name "SecretFlasherManaka" -ErrorAction SilentlyContinue)
foreach ($process in $running) {
    Write-Host "Closing running game process $($process.Id)..."
    [void]$process.CloseMainWindow()
    if (-not $process.WaitForExit(10000)) {
        $process.Kill()
        $process.WaitForExit(5000)
    }
    $process.Dispose()
}

$packageRoot = Join-Path $gameRoot "EscapeFromManaka"
$backupRoot = Join-Path $packageRoot (
    "Backup\Local\" + [DateTime]::UtcNow.ToString("yyyyMMddTHHmmssZ"))
$proxyTarget = Join-Path $gameRoot "winhttp.dll"
if (Test-Path -LiteralPath $proxyTarget) {
    New-Item -ItemType Directory -Force -Path $backupRoot | Out-Null
    Copy-Item -LiteralPath $proxyTarget `
        -Destination (Join-Path $backupRoot "winhttp.dll") -Force
    foreach ($name in @("ReleaseInfo.json", "LauncherState.json", "UpdateHost.exe")) {
        $source = Join-Path $packageRoot $name
        if (Test-Path -LiteralPath $source) {
            Copy-Item -LiteralPath $source `
                -Destination (Join-Path $backupRoot $name) -Force
        }
    }
}

$originalProxy = Join-Path $gameRoot "winhttp_orig.dll"
if (-not (Test-Path -LiteralPath $originalProxy)) {
    Copy-Item -LiteralPath (Join-Path $env:SystemRoot "System32\winhttp.dll") `
        -Destination $originalProxy -Force
}

New-Item -ItemType Directory -Force -Path $packageRoot | Out-Null
foreach ($name in @("Assets", "Checkpoints", "Audio", "Maps", "Fonts")) {
    $managed = Join-Path $packageRoot $name
    if (Test-Path -LiteralPath $managed) {
        $resolved = [IO.Path]::GetFullPath($managed)
        $packagePrefix = [IO.Path]::GetFullPath($packageRoot) +
            [IO.Path]::DirectorySeparatorChar
        if (-not $resolved.StartsWith(
                $packagePrefix,
                [StringComparison]::OrdinalIgnoreCase)) {
            throw "Unsafe managed runtime path: $resolved"
        }
        Remove-Item -LiteralPath $resolved -Recurse -Force
    }
}

Expand-Archive -LiteralPath $runtimeArchive `
    -DestinationPath $packageRoot -Force

$temporaryProxy = $proxyTarget + ".local-new"
Copy-Item -LiteralPath $nativeProxy -Destination $temporaryProxy -Force
if (Test-Path -LiteralPath $proxyTarget) {
    $replaceBackup = $proxyTarget + ".local-previous"
    [IO.File]::Replace(
        $temporaryProxy,
        $proxyTarget,
        $replaceBackup,
        $true)
    Remove-Item -LiteralPath $replaceBackup -Force
} else {
    [IO.File]::Move($temporaryProxy, $proxyTarget)
}

$updateHost = Join-Path $packageRoot "UpdateHost.exe"
$installedPath = Join-Path $packageRoot "installed.json"
$manifest = [ordered]@{
    name = "SecretFlasherManaka Escape from Manaka"
    version = $version
    installedAtUtc = [DateTime]::UtcNow.ToString("O")
    proxySha256 = (Get-FileHash -LiteralPath $proxyTarget -Algorithm SHA256).Hash.ToLowerInvariant()
    updateHostSha256 = (Get-FileHash -LiteralPath $updateHost -Algorithm SHA256).Hash.ToLowerInvariant()
    installationSource = "local-build"
}
[IO.File]::WriteAllText(
    $installedPath,
    ($manifest | ConvertTo-Json -Depth 4) + [Environment]::NewLine,
    [Text.UTF8Encoding]::new($false))

Write-Host "Local build $version installed to:"
Write-Host $gameRoot
Write-Host "Backup:"
Write-Host $backupRoot

if ($Launch) {
    Start-Process -FilePath $gameExe -WorkingDirectory $gameRoot
    Write-Host "Game launched for developer verification."
}
