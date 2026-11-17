[CmdletBinding()]
param(
    [ValidateSet("dev", "stable")]
    [string]$Channel = "dev",

    [string]$GameDir = "D:\Study\SecretFlasherManaka",

    [UInt64]$ReleaseSequence = 1,

    [string]$SigningKeyId = "",

    [string]$SigningKeyPath = "$env:USERPROFILE\.efm\keys\manifest-dev-2026-01-private.pem",

    [string]$SigningCertificatePath = "",

    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$root = Split-Path $PSScriptRoot -Parent
$versionPath = Join-Path $root "VERSION"
$trustRoot = Join-Path $root "deploy\trust"
$defaultKeyIdPath = Join-Path $trustRoot "manifest-key-id.txt"
if ([string]::IsNullOrWhiteSpace($SigningCertificatePath)) {
    $manifestPublicCertificatePath = if ($Channel -eq "stable") {
        Join-Path $trustRoot "manifest-stable-signing-public.der"
    } else {
        Join-Path $trustRoot "manifest-signing-public.der"
    }
} else {
    $manifestPublicCertificatePath = $SigningCertificatePath
}
$version = (Get-Content -LiteralPath $versionPath -Raw).Trim()
if ($version -notmatch '^([0-9]+)\.([0-9]+)\.([0-9]+)$') {
    throw "Invalid version '$version'. Expected LEAD.MAJOR.MINOR."
}
if ([string]::IsNullOrWhiteSpace($SigningKeyId)) {
    if (-not (Test-Path -LiteralPath $defaultKeyIdPath)) {
        throw "Manifest key id file not found: $defaultKeyIdPath"
    }
    $SigningKeyId = (Get-Content -LiteralPath $defaultKeyIdPath -Raw).Trim()
}
if (-not (Test-Path -LiteralPath $SigningKeyPath)) {
    throw "Manifest signing key not found: $SigningKeyPath"
}
if ($ReleaseSequence -lt 1) {
    throw "ReleaseSequence must be at least 1."
}

if (-not $SkipBuild) {
    & (Join-Path $root "build.ps1") -GameDir $GameDir | Out-Host
}

$nativeDll = Join-Path $root "artifacts\winhttp.dll"
if (-not (Test-Path -LiteralPath $nativeDll)) {
    throw "Native DLL not found: $nativeDll"
}

$utcNow = [DateTime]::UtcNow
$buildId = $utcNow.ToString("yyyyMMdd'T'HHmmss'Z'")
$releaseRoot = Join-Path $root "artifacts\releases\$buildId"
$runtimeRoot = Join-Path $releaseRoot "runtime"
$objectRoot = Join-Path $releaseRoot "objects"

foreach ($path in @($releaseRoot, $runtimeRoot, $objectRoot)) {
    if (Test-Path -LiteralPath $path) {
        throw "Release staging path already exists: $path"
    }
    New-Item -ItemType Directory -Force -Path $path | Out-Null
}

function Copy-RuntimeTree {
    param(
        [Parameter(Mandatory)]
        [string]$Source,

        [Parameter(Mandatory)]
        [string]$Destination,

        [Parameter(Mandatory)]
        [string[]]$Extensions,

        [switch]$ExcludeBackups
    )

    if (-not (Test-Path -LiteralPath $Source)) {
        throw "Runtime source path not found: $Source"
    }

    foreach ($file in Get-ChildItem -LiteralPath $Source -Recurse -File) {
        if ($Extensions -notcontains $file.Extension.ToLowerInvariant()) {
            continue
        }
        if ($ExcludeBackups -and $file.Name -match '\.(bak|supply-bak)(\.|$)') {
            continue
        }

        $relative = [System.IO.Path]::GetRelativePath($Source, $file.FullName)
        $target = Join-Path $Destination $relative
        New-Item -ItemType Directory -Force -Path (Split-Path $target -Parent) |
            Out-Null
        Copy-Item -LiteralPath $file.FullName -Destination $target -Force
    }
}

$packageRoot = Join-Path $runtimeRoot "EscapeFromManaka"
New-Item -ItemType Directory -Force -Path $packageRoot | Out-Null
Copy-Item -LiteralPath $nativeDll -Destination (Join-Path $runtimeRoot "winhttp.dll")
Copy-Item -LiteralPath (Join-Path $root "artifacts\UpdateHost.exe") `
    -Destination (Join-Path $packageRoot "UpdateHost.new.exe")
Copy-RuntimeTree -Source (Join-Path $root "assets\equipment") `
    -Destination (Join-Path $packageRoot "Assets") `
    -Extensions @(".json", ".png")
Copy-RuntimeTree -Source (Join-Path $root "assets\checkpoints") `
    -Destination (Join-Path $packageRoot "Checkpoints") `
    -Extensions @(".json", ".csv") `
    -ExcludeBackups
Copy-RuntimeTree -Source (Join-Path $root "assets\maps") `
    -Destination (Join-Path $packageRoot "Maps") `
    -Extensions @(".json", ".png", ".webp")
Copy-RuntimeTree -Source (Join-Path $root "assets\fonts") `
    -Destination (Join-Path $packageRoot "Fonts") `
    -Extensions @(".ttf", ".otf")
Copy-RuntimeTree -Source (Join-Path $root "assets\moaning3") `
    -Destination (Join-Path $packageRoot "Audio") `
    -Extensions @(".json", ".wav")
Copy-Item -LiteralPath (Join-Path $root "README.md") `
    -Destination (Join-Path $packageRoot "README.md")

$releaseInfo = [ordered]@{
    schemaVersion = 1
    version = $version
    channel = $Channel
    buildId = $buildId
    publishedAtUtc = $utcNow.ToString("O")
}
$releaseInfoPath = Join-Path $packageRoot "ReleaseInfo.json"
[System.IO.File]::WriteAllText(
    $releaseInfoPath,
    ($releaseInfo | ConvertTo-Json -Depth 4) + [Environment]::NewLine,
    [System.Text.UTF8Encoding]::new($false))

$allowedRuntimeMarkdown = Join-Path $packageRoot "README.md"
$unexpectedRuntimeMarkdown = @(
    Get-ChildItem -LiteralPath $runtimeRoot -Recurse -File -Filter "*.md" |
        Where-Object { $_.FullName -ne $allowedRuntimeMarkdown }
)
if ($unexpectedRuntimeMarkdown.Count -gt 0) {
    throw ("Runtime release contains development-only Markdown files: " +
        (($unexpectedRuntimeMarkdown | ForEach-Object {
            [System.IO.Path]::GetRelativePath(
                $runtimeRoot,
                $_.FullName)
        }) -join ", "))
}

$gameAssemblyPath = Join-Path $GameDir "GameAssembly.dll"
$gameAssemblyHash = $null
if (Test-Path -LiteralPath $gameAssemblyPath) {
    $gameAssemblyHash = (Get-FileHash -LiteralPath $gameAssemblyPath -Algorithm SHA256).Hash.ToLowerInvariant()
}

$files = @()
$seenHashes = [System.Collections.Generic.HashSet[string]]::new(
    [System.StringComparer]::OrdinalIgnoreCase)
foreach ($file in Get-ChildItem -LiteralPath $runtimeRoot -Recurse -File) {
    $relative = [System.IO.Path]::GetRelativePath($runtimeRoot, $file.FullName).Replace("\", "/")
    $hash = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    $files += [ordered]@{
        path = $relative
        sha256 = $hash
        size = $file.Length
        object = "objects/$hash"
    }

    if ($seenHashes.Add($hash)) {
        Copy-Item -LiteralPath $file.FullName -Destination (Join-Path $objectRoot $hash)
    }
}

$totalBytes = [UInt64]0
foreach ($file in $files) {
    $totalBytes += [UInt64]$file.size
}

$manifest = [ordered]@{
    schemaVersion = 1
    releaseSequence = $ReleaseSequence
    version = $version
    channel = $Channel
    buildId = $buildId
    publishedAtUtc = $utcNow.ToString("O")
    protocolVersion = "EFM-COOP/1"
    signingKeyId = $SigningKeyId
    gameAssemblySha256 = $gameAssemblyHash
    totalFiles = $files.Count
    totalBytes = $totalBytes
    files = $files
}

$manifestPath = Join-Path $releaseRoot "manifest.json"
[System.IO.File]::WriteAllText(
    $manifestPath,
    ($manifest | ConvertTo-Json -Depth 8) + [Environment]::NewLine,
    [System.Text.UTF8Encoding]::new($false))

$manifestHash = (Get-FileHash -LiteralPath $manifestPath -Algorithm SHA256).Hash.ToLowerInvariant()
[System.IO.File]::WriteAllText(
    (Join-Path $releaseRoot "manifest.sha256"),
    "$manifestHash  manifest.json" + [Environment]::NewLine,
    [System.Text.UTF8Encoding]::new($false))

$signaturePath = Join-Path $releaseRoot "manifest.sig"
& openssl dgst -sha256 -sign $SigningKeyPath -out $signaturePath $manifestPath
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $signaturePath)) {
    throw "Manifest signing failed."
}
$publicKeyPem = Join-Path $releaseRoot "manifest-public-key.pem"
& openssl x509 -inform DER -in $manifestPublicCertificatePath `
    -pubkey -noout -out $publicKeyPem
if ($LASTEXITCODE -ne 0) {
    throw "Unable to extract the manifest verification key."
}
& openssl dgst -sha256 -verify $publicKeyPem `
    -signature $signaturePath $manifestPath | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw "Manifest signature does not match the embedded public certificate."
}
Remove-Item -LiteralPath $publicKeyPem -Force

$manifestBytes = [System.IO.File]::ReadAllBytes($manifestPath)
$signatureBytes = [System.IO.File]::ReadAllBytes($signaturePath)
$envelope = [ordered]@{
    schemaVersion = 1
    algorithm = "ES256"
    keyId = $SigningKeyId
    manifest = [Convert]::ToBase64String($manifestBytes)
    signature = [Convert]::ToBase64String($signatureBytes)
}
$envelopePath = Join-Path $releaseRoot "channel-envelope.json"
[System.IO.File]::WriteAllText(
    $envelopePath,
    ($envelope | ConvertTo-Json -Depth 4) + [Environment]::NewLine,
    [System.Text.UTF8Encoding]::new($false))

[pscustomobject]@{
    Version = $version
    Channel = $Channel
    BuildId = $buildId
    ReleaseSequence = $ReleaseSequence
    ManifestPath = $manifestPath
    ManifestSignaturePath = $signaturePath
    EnvelopePath = $envelopePath
    ManifestSha256 = $manifestHash
    ReleaseRoot = $releaseRoot
    RuntimeRoot = $runtimeRoot
    ObjectRoot = $objectRoot
    TotalFiles = $files.Count
    TotalBytes = $totalBytes
} | ConvertTo-Json -Depth 4
