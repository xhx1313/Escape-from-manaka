param(
    [string]$GameDir = $env:EFM_GAME_DIR,
    [string]$UpdateBaseUrl = "",
    [string]$StableBaseUrl = "",
    [string]$DevBaseUrl = ""
)

$ErrorActionPreference = "Stop"
$root = $PSScriptRoot
$zig = Join-Path (Split-Path $root -Parent) "zig-npm\node_modules\@ryoppippi\zig-win32-x64\zig.exe"
$csc = "C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe"
$artifacts = Join-Path $root "artifacts"
$publish = Join-Path $root "publish"
$objectDir = Join-Path $artifacts "native-objects"
$equipmentAssets = Join-Path $root "assets\equipment"
$equipmentCatalog = Join-Path $equipmentAssets "equipment_catalog.json"
$equipmentData = Join-Path $root "native\modules\equipment_inventory_data.inc"
$equipmentZip = Join-Path $artifacts "equipment-assets.zip"
$checkpointAssets = Join-Path $root "assets\checkpoints"
$checkpointZip = Join-Path $artifacts "checkpoint-assets.zip"
$moaningAssets = Join-Path $root "assets\moaning3"
$moaningZip = Join-Path $artifacts "moaning3-assets.zip"
$mapAssets = Join-Path $root "assets\maps"
$mapZip = Join-Path $artifacts "map-assets.zip"
$mapStaging = Join-Path $artifacts "map-assets-staging"
$runtimeAssetsZip = Join-Path $artifacts "runtime-assets.zip"
$runtimeAssetsStaging = Join-Path $artifacts "runtime-assets-staging"
$versionPath = Join-Path $root "VERSION"
$generatedDir = Join-Path $artifacts "generated"
$nativeVersionHeader = Join-Path $generatedDir "mod_version.inc"
$installerVersionSource = Join-Path $generatedDir "ModVersion.g.cs"
$updateHostVersionSource = Join-Path $generatedDir "UpdateHostKeys.g.cs"
$updateHostSource = Join-Path $root "updater\UpdateHost.cs"
$updateHostOutput = Join-Path $artifacts "UpdateHost.exe"
$publishedUpdateHost = Join-Path $publish "UpdateHost.exe"
$updateHostFingerprintPath = Join-Path $artifacts "UpdateHost.cache.fingerprint"
$devLauncherSource = Join-Path $root "developer\DevLauncher.cs"
$devLauncherOutput = Join-Path $artifacts "Start-Dev.exe"
$publishedDevLauncher = Join-Path $publish "Start-Dev.exe"
$trustRoot = Join-Path $root "deploy\trust"
$tlsPinPath = Join-Path $trustRoot "tls-spki-sha256.base64"
$tlsCertificatePath = Join-Path $trustRoot "efm-update-tls.crt"
$manifestKeyIdPath = Join-Path $trustRoot "manifest-key-id.txt"
$manifestPublicCertificatePath = Join-Path $trustRoot "manifest-signing-public.der"
$stableManifestKeyIdPath = Join-Path $trustRoot "manifest-stable-key-id.txt"
$stableManifestPublicCertificatePath = Join-Path $trustRoot "manifest-stable-signing-public.der"
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

$UpdateBaseUrl = Resolve-OpsValue `
    -Value $UpdateBaseUrl `
    -EnvironmentName "EFM_UPDATE_BASE_URL" `
    -ConfigName "UpdateBaseUrl"
$StableBaseUrl = Resolve-OpsValue `
    -Value $StableBaseUrl `
    -EnvironmentName "EFM_STABLE_BASE_URL" `
    -ConfigName "StableBaseUrl"
$DevBaseUrl = Resolve-OpsValue `
    -Value $DevBaseUrl `
    -EnvironmentName "EFM_DEV_BASE_URL" `
    -ConfigName "DevBaseUrl"

if ([string]::IsNullOrWhiteSpace($UpdateBaseUrl)) {
    $UpdateBaseUrl = $StableBaseUrl
}
if ([string]::IsNullOrWhiteSpace($StableBaseUrl)) {
    $StableBaseUrl = $UpdateBaseUrl
}
foreach ($baseUrl in @($UpdateBaseUrl, $StableBaseUrl, $DevBaseUrl)) {
    if ([string]::IsNullOrWhiteSpace($baseUrl)) {
        throw "Update base URLs are required. Set EFM_*_BASE_URL or deploy/ops.local.psd1."
    }
}
$UpdateBaseUrl = $UpdateBaseUrl.TrimEnd("/") + "/"
$StableBaseUrl = $StableBaseUrl.TrimEnd("/") + "/"
$DevBaseUrl = $DevBaseUrl.TrimEnd("/") + "/"
foreach ($baseUrl in @($UpdateBaseUrl, $StableBaseUrl, $DevBaseUrl)) {
    $parsedUrl = $null
    if (-not [Uri]::TryCreate($baseUrl, [UriKind]::Absolute, [ref]$parsedUrl) -or
        $parsedUrl.Scheme -ne "https") {
        throw "Update base URL must be an absolute HTTPS URL."
    }
}

if (-not (Test-Path -LiteralPath $zig)) { throw "Zig compiler not found: $zig" }
if (-not (Test-Path -LiteralPath $csc)) { throw ".NET Framework C# compiler not found: $csc" }

$version = (Get-Content -LiteralPath $versionPath -Raw).Trim()
if ($version -notmatch '^([0-9]+)\.([0-9]+)\.([0-9]+)$') {
    throw "Invalid version '$version'. Expected LEAD.MAJOR.MINOR."
}
$versionParts = $version.Split(".")
$versionLead = [int]$versionParts[0]
$versionMajor = [int]$versionParts[1]
$versionMinor = [int]$versionParts[2]
if ($versionLead -gt 255 -or $versionMajor -gt 255 -or $versionMinor -gt 255) {
    throw "LEAD, MAJOR and MINOR must stay within 0..255 for the native packed ABI."
}
$versionPacked = ($versionLead -shl 16) -bor
    ($versionMajor -shl 8) -bor $versionMinor

New-Item -ItemType Directory -Force -Path $artifacts, $publish, $objectDir, $generatedDir | Out-Null
[System.IO.File]::WriteAllText(
    $nativeVersionHeader,
    "#pragma once`n" +
        "#define MANAKA_MOD_VERSION_STRING L`"$version`"`n" +
        "#define MANAKA_MOD_VERSION_LEAD $versionLead`n" +
        "#define MANAKA_MOD_VERSION_MAJOR $versionMajor`n" +
        "#define MANAKA_MOD_VERSION_MINOR $versionMinor`n" +
        "#define MANAKA_MOD_VERSION_PACKED 0x$($versionPacked.ToString('X8'))u`n",
    [System.Text.UTF8Encoding]::new($false))
[System.IO.File]::WriteAllText(
    $installerVersionSource,
    "namespace ManakaMultiplayer.Setup`n" +
        "{`n" +
        "    internal static class ModVersion`n" +
        "    {`n" +
        "        internal const string Value = `"$version`";`n" +
        "    }`n" +
        "}`n",
    [System.Text.UTF8Encoding]::new($false))

if (-not (Test-Path -LiteralPath $tlsPinPath) -or
    -not (Test-Path -LiteralPath $tlsCertificatePath) -or
    -not (Test-Path -LiteralPath $manifestKeyIdPath) -or
    -not (Test-Path -LiteralPath $manifestPublicCertificatePath) -or
    -not (Test-Path -LiteralPath $stableManifestKeyIdPath) -or
    -not (Test-Path -LiteralPath $stableManifestPublicCertificatePath) -or
    -not (Test-Path -LiteralPath $updateHostSource)) {
    throw "Update host trust files or source are missing."
}
$tlsPin = (Get-Content -LiteralPath $tlsPinPath -Raw).Trim()
$manifestKeyId = (Get-Content -LiteralPath $manifestKeyIdPath -Raw).Trim()
$manifestPublicCertificate = [Convert]::ToBase64String(
    [System.IO.File]::ReadAllBytes($manifestPublicCertificatePath))
$stableManifestKeyId =
    (Get-Content -LiteralPath $stableManifestKeyIdPath -Raw).Trim()
$stableManifestPublicCertificate = [Convert]::ToBase64String(
    [System.IO.File]::ReadAllBytes(
        $stableManifestPublicCertificatePath))
[void][Convert]::FromBase64String($tlsPin)
[void][Convert]::FromBase64String($manifestPublicCertificate)
[void][Convert]::FromBase64String($stableManifestPublicCertificate)
$tlsPublicPem = Join-Path $generatedDir "tls-public-key.pem"
$tlsSpkiDer = Join-Path $generatedDir "tls-spki.der"
& openssl x509 -in $tlsCertificatePath -pubkey -noout -out $tlsPublicPem
if ($LASTEXITCODE -ne 0) { throw "Unable to extract the TLS public key." }
& openssl pkey -pubin -in $tlsPublicPem -outform DER -out $tlsSpkiDer
if ($LASTEXITCODE -ne 0) { throw "Unable to encode the TLS public key." }
$actualTlsPin = [Convert]::ToBase64String(
    [System.Security.Cryptography.SHA256]::Create().ComputeHash(
        [System.IO.File]::ReadAllBytes($tlsSpkiDer)))
if (-not [string]::Equals(
        $tlsPin,
        $actualTlsPin,
        [System.StringComparison]::Ordinal)) {
    throw "TLS SPKI pin does not match deploy/trust/efm-update-tls.crt."
}
[System.IO.File]::WriteAllText(
    $updateHostVersionSource,
    "namespace ManakaMultiplayer.UpdateHost`n" +
        "{`n" +
        "    internal static class UpdateHostKeys`n" +
        "    {`n" +
        "        internal const string UpdateBaseUrl = `"$UpdateBaseUrl`";`n" +
        "        internal const string StableBaseUrl = `"$StableBaseUrl`";`n" +
        "        internal const string DevBaseUrl = `"$DevBaseUrl`";`n" +
        "        internal const string TlsSpkiSha256Base64 = `"$tlsPin`";`n" +
        "        internal const string DevManifestKeyId = `"$manifestKeyId`";`n" +
        "        internal const string DevManifestSigningCertificateBase64 = `"$manifestPublicCertificate`";`n" +
        "        internal const string StableManifestKeyId = `"$stableManifestKeyId`";`n" +
        "        internal const string StableManifestSigningCertificateBase64 = `"$stableManifestPublicCertificate`";`n" +
        "        internal const string UpdateHostVersion = `"$version`";`n" +
        "    }`n" +
        "}`n",
    [System.Text.UTF8Encoding]::new($false))

if (-not (Test-Path -LiteralPath $equipmentCatalog)) {
    throw "Equipment catalog not found: $equipmentCatalog"
}
& (Join-Path $root "tools\verify_moaning_assets.ps1") `
    -AssetRoot $moaningAssets
$python = (Get-Command python -ErrorAction Stop).Source
& $python (Join-Path $root "tools\generate_equipment_data.py") `
    $equipmentCatalog $equipmentData
if ($LASTEXITCODE -ne 0) { throw "Equipment data generation failed." }

Compress-Archive -Path (Join-Path $equipmentAssets "*") `
    -DestinationPath $equipmentZip -CompressionLevel Optimal -Force
Compress-Archive -Path (Join-Path $checkpointAssets "*") `
    -DestinationPath $checkpointZip -CompressionLevel Optimal -Force
Compress-Archive -Path (Join-Path $moaningAssets "*") `
    -DestinationPath $moaningZip -CompressionLevel Optimal -Force

if (Test-Path -LiteralPath $mapStaging) {
    $resolvedStaging = [System.IO.Path]::GetFullPath($mapStaging)
    $resolvedArtifacts = [System.IO.Path]::GetFullPath($artifacts)
    if (-not $resolvedStaging.StartsWith(
            $resolvedArtifacts + [System.IO.Path]::DirectorySeparatorChar,
            [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Map staging directory escaped the artifacts directory."
    }
    Remove-Item -LiteralPath $resolvedStaging -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $mapStaging | Out-Null
$mapRuntimeExtensions = @(".png", ".webp", ".json")
foreach ($mapFile in Get-ChildItem -LiteralPath $mapAssets -Recurse -File |
        Where-Object { $_.Extension -in $mapRuntimeExtensions }) {
    $relativePath = [System.IO.Path]::GetRelativePath(
        $mapAssets,
        $mapFile.FullName)
    $destination = Join-Path $mapStaging $relativePath
    New-Item -ItemType Directory -Force `
        -Path (Split-Path $destination -Parent) | Out-Null
    Copy-Item -LiteralPath $mapFile.FullName `
        -Destination $destination -Force
}
Compress-Archive -Path (Join-Path $mapStaging "*") `
    -DestinationPath $mapZip -CompressionLevel Optimal -Force

$minhookRoot = Join-Path $root "native\minhook"
$minhookSourceFiles = @(
    (Join-Path $minhookRoot "src\buffer.c"),
    (Join-Path $minhookRoot "src\hook.c"),
    (Join-Path $minhookRoot "src\trampoline.c"),
    (Join-Path $minhookRoot "src\hde\hde64.c")
)
$minhookObjects = @()
foreach ($sourceFile in $minhookSourceFiles) {
    $objectFile = Join-Path $objectDir ((Split-Path $sourceFile -Leaf) + ".o")
    & $zig cc -target x86_64-windows-gnu -O2 -c `
        -I (Join-Path $minhookRoot "include") `
        -I (Join-Path $minhookRoot "src") `
        -I (Join-Path $minhookRoot "src\hde") `
        $sourceFile -o $objectFile
    if ($LASTEXITCODE -ne 0) { throw "MinHook build failed: $sourceFile" }
    $minhookObjects += $objectFile
}

& $zig c++ -target x86_64-windows-gnu -std=c++17 -O2 -shared -static `
    -DUNICODE -D_UNICODE `
    -I $generatedDir `
    -I (Join-Path $minhookRoot "include") `
    -I (Join-Path $root "native\third_party") `
    (Join-Path $root "native\Native.cpp") (Join-Path $root "native\winhttp.def") `
    $minhookObjects `
    -o (Join-Path $artifacts "winhttp.dll") `
    -lws2_32 -luser32 -lgdi32 -lcomctl32
if ($LASTEXITCODE -ne 0) { throw "Native DLL build failed." }

$updateHostFingerprint = @(
    "schema=2",
    $version,
    (Get-FileHash -LiteralPath $updateHostSource -Algorithm SHA256).Hash,
    (Get-FileHash -LiteralPath $updateHostVersionSource -Algorithm SHA256).Hash,
    (Get-FileHash -LiteralPath $csc -Algorithm SHA256).Hash
) -join "|"
$cachedUpdateHostFingerprint = if (Test-Path -LiteralPath $updateHostFingerprintPath) {
    (Get-Content -LiteralPath $updateHostFingerprintPath -Raw).Trim()
} else {
    ""
}
if (-not (Test-Path -LiteralPath $updateHostOutput) -or
    $cachedUpdateHostFingerprint -ne $updateHostFingerprint) {
    & $csc /nologo /target:winexe /platform:x64 /optimize+ /debug- `
        /out:$updateHostOutput `
        /codepage:65001 `
        /reference:System.dll /reference:System.Core.dll /reference:System.Security.dll `
        /reference:System.Web.Extensions.dll `
        $updateHostSource $updateHostVersionSource
    if ($LASTEXITCODE -ne 0) { throw "Update host build failed." }
    [System.IO.File]::WriteAllText(
        $updateHostFingerprintPath,
        $updateHostFingerprint,
        [System.Text.UTF8Encoding]::new($false))
}
Copy-Item -LiteralPath $updateHostOutput -Destination $publishedUpdateHost -Force

& $csc /nologo /target:winexe /platform:x64 /optimize+ /debug- `
    /out:$devLauncherOutput `
    /codepage:65001 `
    /reference:System.dll /reference:System.Core.dll /reference:System.Security.dll `
    /reference:System.Drawing.dll /reference:System.Windows.Forms.dll `
    $devLauncherSource
if ($LASTEXITCODE -ne 0) { throw "Developer launcher build failed." }
Copy-Item -LiteralPath $devLauncherOutput -Destination $publishedDevLauncher -Force

if (Test-Path -LiteralPath $runtimeAssetsStaging) {
    $resolvedRuntimeStaging = [System.IO.Path]::GetFullPath(
        $runtimeAssetsStaging)
    $resolvedArtifacts = [System.IO.Path]::GetFullPath($artifacts)
    if (-not $resolvedRuntimeStaging.StartsWith(
            $resolvedArtifacts + [System.IO.Path]::DirectorySeparatorChar,
            [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Runtime staging directory escaped the artifacts directory."
    }
    Remove-Item -LiteralPath $resolvedRuntimeStaging -Recurse -Force
}
$runtimePackageRoot = Join-Path $runtimeAssetsStaging "EscapeFromManaka"
New-Item -ItemType Directory -Force -Path $runtimePackageRoot | Out-Null
Expand-Archive -LiteralPath $equipmentZip `
    -DestinationPath (Join-Path $runtimePackageRoot "Assets") -Force
Expand-Archive -LiteralPath $checkpointZip `
    -DestinationPath (Join-Path $runtimePackageRoot "Checkpoints") -Force
Expand-Archive -LiteralPath $moaningZip `
    -DestinationPath (Join-Path $runtimePackageRoot "Audio") -Force
Expand-Archive -LiteralPath $mapZip `
    -DestinationPath (Join-Path $runtimePackageRoot "Maps") -Force
New-Item -ItemType Directory -Force `
    -Path (Join-Path $runtimePackageRoot "Fonts") | Out-Null
Copy-Item -Path (Join-Path $root "assets\fonts\*") `
    -Destination (Join-Path $runtimePackageRoot "Fonts") -Force
Copy-Item -LiteralPath (Join-Path $root "README.md") `
    -Destination (Join-Path $runtimePackageRoot "README.md") -Force
Copy-Item -LiteralPath $updateHostOutput `
    -Destination (Join-Path $runtimePackageRoot "UpdateHost.exe") -Force
$installerBuildId = [DateTime]::UtcNow.ToString("yyyyMMdd'T'HHmmss'Z'")
$installerReleaseInfo = [ordered]@{
    schemaVersion = 1
    version = $version
    channel = "stable"
    buildId = "installer-$installerBuildId"
    publishedAtUtc = [DateTime]::UtcNow.ToString("O")
}
[System.IO.File]::WriteAllText(
    (Join-Path $runtimePackageRoot "ReleaseInfo.json"),
    ($installerReleaseInfo | ConvertTo-Json -Depth 4) +
        [Environment]::NewLine,
    [System.Text.UTF8Encoding]::new($false))
$installerLauncherState = [ordered]@{
    channel = "stable"
    version = $version
    buildId = "installer-$installerBuildId"
    releaseSequence = 0
}
[System.IO.File]::WriteAllText(
    (Join-Path $runtimePackageRoot "LauncherState.json"),
    ($installerLauncherState | ConvertTo-Json -Depth 4) +
        [Environment]::NewLine,
    [System.Text.UTF8Encoding]::new($false))
$runtimeMarkdownFiles = @(
    Get-ChildItem -LiteralPath $runtimePackageRoot -Recurse -File `
        -Filter "*.md"
)
$unexpectedRuntimeMarkdown = @(
    $runtimeMarkdownFiles | Where-Object {
        $_.FullName -ne (Join-Path $runtimePackageRoot "README.md")
    }
)
if ($unexpectedRuntimeMarkdown.Count -gt 0) {
    throw ("Runtime package contains development-only Markdown files: " +
        (($unexpectedRuntimeMarkdown | ForEach-Object {
            [System.IO.Path]::GetRelativePath(
                $runtimePackageRoot,
                $_.FullName)
        }) -join ", "))
}
if (Test-Path -LiteralPath $runtimeAssetsZip) {
    Remove-Item -LiteralPath $runtimeAssetsZip -Force
}
Compress-Archive -Path (Join-Path $runtimePackageRoot "*") `
    -DestinationPath $runtimeAssetsZip -CompressionLevel Optimal

$installerSource = Join-Path $root "installer\Installer.cs"
$installerOutput = Join-Path $publish "SecretFlasherManaka.EscapeFromManaka.Setup.exe"
$nativeProxy = Join-Path $artifacts "winhttp.dll"
$resourceArguments = @(
    "/resource:$nativeProxy,winhttp.dll",
    "/resource:$updateHostOutput,UpdateHost.exe",
    "/resource:$runtimeAssetsZip,runtime-assets.zip"
)
& $csc /nologo /target:winexe /platform:x64 /optimize+ /debug- `
    /out:$installerOutput `
    /reference:System.dll /reference:System.Core.dll `
    /reference:System.Drawing.dll /reference:System.Windows.Forms.dll `
    /reference:System.IO.Compression.dll /reference:System.IO.Compression.FileSystem.dll `
    @resourceArguments `
    $installerSource $installerVersionSource
if ($LASTEXITCODE -ne 0) { throw "Installer build failed." }

$smokeSource = Join-Path $root "test\ProxySmoke.cs"
$smokeOutput = Join-Path $artifacts "ProxySmoke.exe"
& $csc /nologo /target:exe /platform:x64 /optimize+ /debug- `
    /out:$smokeOutput /reference:System.dll $smokeSource
if ($LASTEXITCODE -ne 0) { throw "Proxy smoke-test build failed." }

$windowProbeSource = Join-Path $root "test\WindowProbe.cs"
$windowProbeOutput = Join-Path $artifacts "WindowProbe.exe"
& $csc /nologo /target:exe /platform:x64 /optimize+ /debug- `
    /out:$windowProbeOutput /reference:System.dll $windowProbeSource
if ($LASTEXITCODE -ne 0) { throw "Window probe build failed." }

$hash = (Get-FileHash -LiteralPath $installerOutput -Algorithm SHA256).Hash
Write-Host "Built installer: $installerOutput"
Write-Host "SHA256: $hash"
