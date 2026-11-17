[CmdletBinding()]
param(
    [ValidateSet("show", "set", "minor", "major")]
    [string]$Action = "show",

    [string]$Version
)

$ErrorActionPreference = "Stop"
$root = Split-Path $PSScriptRoot -Parent
$versionPath = Join-Path $root "VERSION"

function Read-ModVersion {
    if (-not (Test-Path -LiteralPath $versionPath)) {
        throw "Version file not found: $versionPath"
    }

    $value = (Get-Content -LiteralPath $versionPath -Raw).Trim()
    if ($value -notmatch '^([0-9]+)\.([0-9]+)\.([0-9]+)$') {
        throw "Invalid version '$value'. Expected LEAD.MAJOR.MINOR."
    }
    return $value
}

function Write-ModVersion([string]$value) {
    [System.IO.File]::WriteAllText(
        $versionPath,
        $value + [Environment]::NewLine,
        [System.Text.UTF8Encoding]::new($false))
}

$current = Read-ModVersion
$next = $current

switch ($Action) {
    "show" {
    }
    "set" {
        if ([string]::IsNullOrWhiteSpace($Version) -or
            $Version -notmatch '^([0-9]+)\.([0-9]+)\.([0-9]+)$') {
            throw "Set requires an explicit version in LEAD.MAJOR.MINOR form."
        }
        $next = $Version
    }
    "minor" {
        $parts = $current.Split(".")
        $next = "$($parts[0]).$($parts[1]).$([UInt64]$parts[2] + 1)"
    }
    "major" {
        $parts = $current.Split(".")
        $next = "$($parts[0]).$([UInt64]$parts[1] + 1).0"
    }
}

$parts = $next.Split(".")
if ([UInt64]$parts[0] -gt 255 -or
    [UInt64]$parts[1] -gt 255 -or
    [UInt64]$parts[2] -gt 255) {
    throw "LEAD, MAJOR and MINOR must stay within 0..255 for the native packed ABI."
}

if ($next -ne $current) {
    Write-ModVersion $next
}

Write-Output $next
