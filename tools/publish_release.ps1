[CmdletBinding()]
param(
    [ValidateSet("dev", "stable")]
    [string]$Channel = "dev",

    [ValidateSet("none", "minor", "major")]
    [string]$Bump = "minor",

    [string]$Server = $env:EFM_DEPLOY_SERVER,

    [string]$ServerRoot = "/srv/efm/public",

    [string]$KeyPath = "$env:USERPROFILE\.ssh\efm_deploy_v2",

    [string]$SigningKeyPath = "",

    [string]$SigningKeyId = "",

    [string]$GameDir = $env:EFM_GAME_DIR,

    [switch]$SkipBuild,

    [switch]$LegacyDevBootstrap
)

$ErrorActionPreference = "Stop"
$root = Split-Path $PSScriptRoot -Parent
$versionScript = Join-Path $PSScriptRoot "version.ps1"
$prepareScript = Join-Path $PSScriptRoot "prepare_release.ps1"
$trustRoot = Join-Path $root "deploy\trust"
$opsConfigPath = Join-Path $root "deploy\ops.local.psd1"
$opsConfig = if (Test-Path -LiteralPath $opsConfigPath) {
    Import-PowerShellDataFile -LiteralPath $opsConfigPath
} else {
    @{}
}

function Resolve-OpsValue {
    param(
        [string]$Value,
        [string]$EnvironmentName,
        [string]$ConfigName
    )

    if ([string]::IsNullOrWhiteSpace($Value)) {
        $Value = [Environment]::GetEnvironmentVariable($EnvironmentName)
    }
    if ([string]::IsNullOrWhiteSpace($Value) -and
        $opsConfig.ContainsKey($ConfigName)) {
        $Value = [string]$opsConfig[$ConfigName]
    }
    return $Value
}

$Server = Resolve-OpsValue `
    -Value $Server `
    -EnvironmentName "EFM_DEPLOY_SERVER" `
    -ConfigName "DeployServer"
if ([string]::IsNullOrWhiteSpace($Server)) {
    throw "Deployment server is required. Set EFM_DEPLOY_SERVER or deploy/ops.local.psd1."
}
$updateBaseUrl = Resolve-OpsValue `
    -Value "" `
    -EnvironmentName "EFM_UPDATE_BASE_URL" `
    -ConfigName "UpdateBaseUrl"
$devBaseUrl = Resolve-OpsValue `
    -Value "" `
    -EnvironmentName "EFM_DEV_BASE_URL" `
    -ConfigName "DevBaseUrl"
if ([string]::IsNullOrWhiteSpace($updateBaseUrl) -or
    [string]::IsNullOrWhiteSpace($devBaseUrl)) {
    throw "Update base URLs are required. Set EFM_*_BASE_URL or deploy/ops.local.psd1."
}
$updateBaseUrl = $updateBaseUrl.TrimEnd("/") + "/"
$devBaseUrl = $devBaseUrl.TrimEnd("/") + "/"

if ([string]::IsNullOrWhiteSpace($SigningKeyPath)) {
    $SigningKeyPath = if ($Channel -eq "stable") {
        "$env:USERPROFILE\.efm\keys\manifest-stable-2026-01-private.pem"
    } else {
        "$env:USERPROFILE\.efm\keys\manifest-dev-2026-01-private.pem"
    }
}
$signingCertificatePath = if ($Channel -eq "stable") {
    Join-Path $trustRoot "manifest-stable-signing-public.der"
} else {
    Join-Path $trustRoot "manifest-signing-public.der"
}
if ([string]::IsNullOrWhiteSpace($SigningKeyId)) {
    $keyIdPath = if ($Channel -eq "stable") {
        Join-Path $trustRoot "manifest-stable-key-id.txt"
    } else {
        Join-Path $trustRoot "manifest-key-id.txt"
    }
    $SigningKeyId = (Get-Content -LiteralPath $keyIdPath -Raw).Trim()
}

if (-not (Test-Path -LiteralPath $KeyPath)) {
    throw "Deployment SSH key not found: $KeyPath"
}
if (-not (Test-Path -LiteralPath $SigningKeyPath)) {
    throw "Manifest signing key not found: $SigningKeyPath"
}

$sshBase = @(
    "-i", $KeyPath,
    "-o", "BatchMode=yes",
    "-o", "StrictHostKeyChecking=accept-new"
)

if ($Channel -eq "dev" -and -not $LegacyDevBootstrap) {
    $remoteDataRoot = "$ServerRoot/dev"
    $remoteChannelsRoot = "$remoteDataRoot/channels"
    $remoteObjectsRoot = "$remoteDataRoot/objects"
    $remoteReleaseRoot = "$remoteDataRoot/releases/dev"
    $signedChannelUrl =
        "${devBaseUrl}channels/dev.json"
    $objectUrlPrefix =
        "${devBaseUrl}objects/"
} else {
    $remoteDataRoot = $ServerRoot
    $remoteChannelsRoot = "$ServerRoot/channels"
    $remoteObjectsRoot = "$ServerRoot/objects"
    $remoteReleaseRoot = if ($Channel -eq "dev") {
        "$ServerRoot/releases/dev"
    } else {
        "$ServerRoot/releases/stable"
    }
    $signedChannelUrl = if ($Channel -eq "dev") {
        "${updateBaseUrl}channels/dev.json"
    } else {
        "${updateBaseUrl}channels/stable.json"
    }
    $objectUrlPrefix = "${updateBaseUrl}objects/"
}

if ($Bump -ne "none") {
    & $versionScript -Action $Bump | Out-Null
}
$versionOutput = & $versionScript -Action show
$version = ($versionOutput | Select-Object -Last 1).ToString().Trim()

& ssh @sshBase $Server "install -d -m 755 '$remoteObjectsRoot' '$remoteChannelsRoot' '$remoteReleaseRoot' '$ServerRoot/staging'"
if ($LASTEXITCODE -ne 0) {
    throw "Unable to prepare remote release directories."
}

$remoteSequenceText = & ssh @sshBase $Server "cat '$remoteChannelsRoot/$Channel.sequence' 2>/dev/null || echo 0"
if ($LASTEXITCODE -ne 0) {
    throw "Unable to read the remote release sequence."
}
$remoteSequence = [UInt64](($remoteSequenceText | Select-Object -Last 1).Trim())
$releaseSequence = $remoteSequence + 1

$storageText = & ssh @sshBase $Server "df -Pk '$ServerRoot' | tail -1; du -sk '$remoteObjectsRoot'"
if ($LASTEXITCODE -ne 0 -or $storageText.Count -lt 2) {
    throw "Unable to inspect remote storage capacity."
}
$diskFields = @(($storageText[0].Trim() -split '\s+'))
$objectFields = @(($storageText[1].Trim() -split '\s+'))
$remoteFreeBytes = [UInt64]$diskFields[3] * 1024
$remoteObjectBytes = [UInt64]$objectFields[0] * 1024
$minimumFreeBytes = [UInt64]8 * 1024 * 1024 * 1024
$maximumObjectBytes = [UInt64]20 * 1024 * 1024 * 1024
if ($remoteFreeBytes -lt $minimumFreeBytes) {
    throw "Remote filesystem has less than 8 GiB free; publication stopped."
}
if ($remoteObjectBytes -ge $maximumObjectBytes) {
    throw "Remote EFM object store has reached its 20 GiB logical limit."
}

$prepareArguments = @{
    Channel = $Channel
    GameDir = $GameDir
    ReleaseSequence = $releaseSequence
    SigningKeyPath = $SigningKeyPath
    SigningKeyId = $SigningKeyId
    SigningCertificatePath = $signingCertificatePath
}
if ($SkipBuild) {
    $prepareArguments.SkipBuild = $true
}
$releaseJson = & $prepareScript @prepareArguments
$release = ($releaseJson | ConvertFrom-Json)
$buildId = [string]$release.BuildId
$manifestPath = [string]$release.ManifestPath
$signaturePath = [string]$release.ManifestSignaturePath
$envelopePath = [string]$release.EnvelopePath
$objectRoot = [string]$release.ObjectRoot

$remoteHashText = & ssh @sshBase $Server "find '$remoteObjectsRoot' -maxdepth 1 -type f -printf '%f\n' 2>/dev/null || true"
if ($LASTEXITCODE -ne 0) {
    throw "Unable to inspect remote update objects."
}
$remoteHashes = [System.Collections.Generic.HashSet[string]]::new(
    [System.StringComparer]::OrdinalIgnoreCase)
foreach ($line in $remoteHashText) {
    if (-not [string]::IsNullOrWhiteSpace($line)) {
        [void]$remoteHashes.Add($line.Trim())
    }
}

$missingFiles = @(Get-ChildItem -LiteralPath $objectRoot -File |
    Where-Object { -not $remoteHashes.Contains($_.Name) })

if ($missingFiles.Count -gt 0) {
    $archivePath = Join-Path $release.ReleaseRoot "missing-objects.tar.gz"
    $missingStage = Join-Path $release.ReleaseRoot "missing-objects"
    New-Item -ItemType Directory -Force -Path $missingStage | Out-Null
    foreach ($file in $missingFiles) {
        Copy-Item -LiteralPath $file.FullName `
            -Destination (Join-Path $missingStage $file.Name)
    }
    $archiveArguments = @(
        "-czf", $archivePath,
        "-C", $missingStage,
        "."
    )
    & tar @archiveArguments
    if ($LASTEXITCODE -ne 0) {
        throw "Unable to create the missing-object archive."
    }

    $remoteArchive = "/tmp/efm-$buildId-objects.tar.gz"
    & scp @sshBase $archivePath "${Server}:$remoteArchive"
    if ($LASTEXITCODE -ne 0) {
        throw "Object archive upload failed."
    }
    & ssh @sshBase $Server "tar -xzf '$remoteArchive' -C '$remoteObjectsRoot' && rm -f '$remoteArchive'"
    if ($LASTEXITCODE -ne 0) {
        throw "Remote object extraction failed."
    }
    Remove-Item -LiteralPath $missingStage -Recurse -Force
    Remove-Item -LiteralPath $archivePath -Force
}

$remoteManifestTemp = "/tmp/efm-$buildId-manifest.json"
$remoteSignatureTemp = "/tmp/efm-$buildId-manifest.sig"
$remoteEnvelopeTemp = "/tmp/efm-$buildId-envelope.json"
& scp @sshBase $manifestPath "${Server}:$remoteManifestTemp"
if ($LASTEXITCODE -ne 0) {
    throw "Manifest upload failed."
}
& scp @sshBase $signaturePath "${Server}:$remoteSignatureTemp"
if ($LASTEXITCODE -ne 0) {
    throw "Manifest signature upload failed."
}
& scp @sshBase $envelopePath "${Server}:$remoteEnvelopeTemp"
if ($LASTEXITCODE -ne 0) {
    throw "Channel envelope upload failed."
}
& ssh @sshBase $Server "install -m 644 '$remoteManifestTemp' '$remoteReleaseRoot/$buildId.json' && install -m 644 '$remoteSignatureTemp' '$remoteReleaseRoot/$buildId.sig' && install -m 644 '$remoteEnvelopeTemp' '$remoteReleaseRoot/$buildId.envelope.json' && install -m 644 '$remoteEnvelopeTemp' '$remoteChannelsRoot/$Channel.json.next' && mv '$remoteChannelsRoot/$Channel.json.next' '$remoteChannelsRoot/$Channel.json' && printf '%s\n' '$releaseSequence' > '$remoteChannelsRoot/$Channel.sequence.next' && mv '$remoteChannelsRoot/$Channel.sequence.next' '$remoteChannelsRoot/$Channel.sequence' && rm -f '$remoteManifestTemp' '$remoteSignatureTemp' '$remoteEnvelopeTemp'"
if ($LASTEXITCODE -ne 0) {
    throw "Signed manifest publication failed."
}

$winhttpRecord = (Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json).files |
    Where-Object { $_.path -eq "winhttp.dll" } |
    Select-Object -First 1
if ($null -eq $winhttpRecord) {
    throw "Released manifest does not contain winhttp.dll."
}

[pscustomobject]@{
    Version = $version
    Channel = $Channel
    BuildId = $buildId
    ReleaseSequence = $releaseSequence
    UploadedObjects = $missingFiles.Count
    ManifestSha256 = $release.ManifestSha256
    WinHttpSha256 = $winhttpRecord.sha256
    SignedChannelUrl = $signedChannelUrl
    WinHttpObjectUrl = "$objectUrlPrefix$($winhttpRecord.sha256)"
} | ConvertTo-Json -Depth 4
