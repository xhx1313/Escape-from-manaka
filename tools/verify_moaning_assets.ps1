param(
    [string]$AssetRoot = ""
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($AssetRoot)) {
    $AssetRoot = Join-Path (Split-Path $PSScriptRoot -Parent) "assets\moaning3"
}
$AssetRoot = [System.IO.Path]::GetFullPath($AssetRoot)
$packRoot = Join-Path $AssetRoot "Moaning3\ryuzen-nikomi-2"
$expected = [ordered]@{
    gasping = 15
    low = 41
    medium = 35
    high = 35
    peak = 8
    orgasm = 6
}

if (-not (Test-Path -LiteralPath $packRoot -PathType Container)) {
    throw "Moaning3 audio pack directory not found: $packRoot"
}

$allFiles = @()
foreach ($category in $expected.Keys) {
    $categoryRoot = Join-Path $packRoot $category
    if (-not (Test-Path -LiteralPath $categoryRoot -PathType Container)) {
        throw "Missing Moaning3 category directory: $categoryRoot"
    }
    $files = @(Get-ChildItem -LiteralPath $categoryRoot -File -Filter *.wav)
    if ($files.Count -ne $expected[$category]) {
        throw (
            "Moaning3 category '$category' has $($files.Count) WAV files; " +
            "expected $($expected[$category])."
        )
    }
    foreach ($file in $files) {
        $stream = [System.IO.File]::OpenRead($file.FullName)
        try {
            $reader = New-Object System.IO.BinaryReader($stream)
            $riff = [System.Text.Encoding]::ASCII.GetString($reader.ReadBytes(4))
            [void]$reader.ReadUInt32()
            $wave = [System.Text.Encoding]::ASCII.GetString($reader.ReadBytes(4))
            if ($riff -ne "RIFF" -or $wave -ne "WAVE") {
                throw "Unsupported WAV container: $($file.FullName)"
            }

            $audioFormat = -1
            $channels = 0
            $sampleRate = 0
            $bitsPerSample = 0
            $hasData = $false
            while ($stream.Position + 8 -le $stream.Length) {
                $chunkId = [System.Text.Encoding]::ASCII.GetString(
                    $reader.ReadBytes(4)
                )
                $chunkSize = [int64]$reader.ReadUInt32()
                $chunkStart = $stream.Position
                if ($chunkId -eq "fmt ") {
                    $audioFormat = $reader.ReadUInt16()
                    $channels = $reader.ReadUInt16()
                    $sampleRate = $reader.ReadUInt32()
                    [void]$reader.ReadUInt32()
                    [void]$reader.ReadUInt16()
                    $bitsPerSample = $reader.ReadUInt16()
                    if ($audioFormat -eq 0xFFFE -and $chunkSize -ge 40) {
                        $stream.Position = $chunkStart + 24
                        $audioFormat = $reader.ReadUInt16()
                    }
                }
                elseif ($chunkId -eq "data") {
                    if ($chunkSize -le 0) {
                        throw "Empty WAV data chunk: $($file.FullName)"
                    }
                    $hasData = $true
                }

                $next = $chunkStart + $chunkSize
                if (($chunkSize -band 1) -ne 0) {
                    ++$next
                }
                if ($next -le $chunkStart -or $next -gt $stream.Length) {
                    break
                }
                $stream.Position = $next
            }

            if (-not $hasData) {
                throw "WAV data chunk not found: $($file.FullName)"
            }
            if ($audioFormat -ne 1 -and $audioFormat -ne 3) {
                throw (
                    "Unsupported WAV encoding $audioFormat in " +
                    "$($file.FullName)"
                )
            }
            if ($bitsPerSample -notin @(16, 24, 32)) {
                throw (
                    "Unsupported WAV bit depth $bitsPerSample in " +
                    "$($file.FullName)"
                )
            }
            if ($channels -lt 1 -or $channels -gt 2 -or
                $sampleRate -lt 8000 -or $sampleRate -gt 192000) {
                throw "Unsupported WAV layout: $($file.FullName)"
            }
            if ($audioFormat -eq 3 -and $bitsPerSample -ne 32) {
                throw "Float WAV must use 32-bit samples: $($file.FullName)"
            }
        }
        finally {
            $stream.Dispose()
        }
        $allFiles += $file
    }
}

if ($allFiles.Count -ne 140) {
    throw "Moaning3 pack has $($allFiles.Count) WAV files; expected 140."
}

$totalBytes = ($allFiles | Measure-Object Length -Sum).Sum
if ($totalBytes -ne 60638352) {
    throw "Moaning3 pack size is $totalBytes bytes; expected 60638352."
}

Write-Output (
    "Verified Moaning3 audio pack: $($allFiles.Count) WAV files, " +
    "$totalBytes bytes."
)
