<#
.SYNOPSIS
  Sync ChillRound fonts into this downstream repo in a reproducible way.

.DESCRIPTION
  Downloads the ChillRound release ZIP, extracts the two files you use and
  copies them into skeleton/SYSTEM/res with the filenames expected by NextUI.

  Mapping (as per your current convention):
    ChillRoundM.otf -> skeleton/SYSTEM/res/BPreplayBold-unhinted.otf
    ChillRoundM.otf -> skeleton/SYSTEM/res/BPreplayBold.otf
    ChillRoundM.ttf -> skeleton/SYSTEM/res/font1.ttf
    ChillRound.ttf  -> skeleton/SYSTEM/res/font2.ttf

  Optional subset:
    If you set -Subset and have python + fonttools available, this script will
    subset ChillRound.ttf using characters found in: workspace/i18n/locales/zh_CN.lang

.NOTES
  This script is written for Windows PowerShell.
#>

param(
  [string]$Url = "https://github.com/Warren2060/ChillRound/releases/download/v1.800/ChillRoundM_v1.800.zip",
  [string]$OutDir = "$PSScriptRoot/.cache/fonts",
  [switch]$Force,
  [switch]$Subset
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$zipPath = Join-Path $OutDir "ChillRound.zip"
$tmpExtract = Join-Path $OutDir "extract"

$dstRes = Join-Path $repoRoot "skeleton\SYSTEM\res"
$dstOtf = Join-Path $dstRes "BPreplayBold-unhinted.otf"
$dstOtfAlias = Join-Path $dstRes "BPreplayBold.otf"
$dstTtfBold = Join-Path $dstRes "font1.ttf"
$dstTtfRegular = Join-Path $dstRes "font2.ttf"

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
New-Item -ItemType Directory -Force -Path $tmpExtract | Out-Null

if ($Force -or -not (Test-Path $zipPath)) {
  Write-Host "[font_sync] Downloading: $Url"
  Invoke-WebRequest -Uri $Url -OutFile $zipPath
}

# Clean extract dir
Get-ChildItem -Path $tmpExtract -Force -ErrorAction SilentlyContinue | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $tmpExtract | Out-Null

Write-Host "[font_sync] Extracting zip..."
Expand-Archive -Path $zipPath -DestinationPath $tmpExtract -Force

# Locate files (zip folder layout might change, so search)
$srcOtf = Get-ChildItem -Path $tmpExtract -Recurse -File -Filter "ChillRoundM.otf" | Select-Object -First 1

# Prefer distinct weights if both exist.
$srcTtfBold = Get-ChildItem -Path $tmpExtract -Recurse -File -Filter "ChillRoundM.ttf" | Select-Object -First 1
$srcTtfRegular = Get-ChildItem -Path $tmpExtract -Recurse -File -Filter "ChillRound.ttf" | Select-Object -First 1

# Fall back so the script still works if only one TTF is present.
if (-not $srcTtfBold -and $srcTtfRegular) { $srcTtfBold = $srcTtfRegular }
if (-not $srcTtfRegular -and $srcTtfBold) { $srcTtfRegular = $srcTtfBold }

if (-not $srcOtf) { throw "ChillRoundM.otf not found inside zip" }
if (-not $srcTtfBold -or -not $srcTtfRegular) { throw "ChillRound TTF fonts not found inside zip" }

New-Item -ItemType Directory -Force -Path $dstRes | Out-Null

Write-Host "[font_sync] Copy: $($srcOtf.FullName) -> $dstOtf"
Copy-Item -Force $srcOtf.FullName $dstOtf

Write-Host "[font_sync] Copy: $($srcOtf.FullName) -> $dstOtfAlias"
Copy-Item -Force $srcOtf.FullName $dstOtfAlias

Write-Host "[font_sync] Copy (bold): $($srcTtfBold.FullName) -> $dstTtfBold"
Copy-Item -Force $srcTtfBold.FullName $dstTtfBold

Write-Host "[font_sync] Copy (regular): $($srcTtfRegular.FullName) -> $dstTtfRegular"
Copy-Item -Force $srcTtfRegular.FullName $dstTtfRegular

if ($Subset) {
  $langFile = Join-Path $repoRoot "workspace\i18n\locales\zh_CN.lang"
  if (-not (Test-Path $langFile)) {
    throw "Language file not found: $langFile"
  }

  $charsetTxt = Join-Path $OutDir "charset.txt"

  Write-Host "[font_sync] Building charset from zh_CN.lang -> $charsetTxt"

  # Collect all characters from values in key=value lines.
  $chars = New-Object System.Collections.Generic.HashSet[string]
  Get-Content -Path $langFile -Encoding UTF8 | ForEach-Object {
    $line = $_.Trim()
    if ($line.Length -eq 0 -or $line.StartsWith('#')) { return }
    $idx = $line.IndexOf('=')
    if ($idx -lt 0) { return }
    $val = $line.Substring($idx + 1)
    foreach ($r in $val.ToCharArray()) {
      $null = $chars.Add([string]$r)
    }
  }

  # Always include ASCII printable range.
  32..126 | ForEach-Object { $null = $chars.Add([char]$_) }

  $sorted = $chars | Sort-Object
  [IO.File]::WriteAllText($charsetTxt, ($sorted -join ""), [Text.Encoding]::UTF8)

  # Subset using fonttools if available.
  Write-Host "[font_sync] Subsetting font1.ttf/font2.ttf (requires python + fonttools/pyftsubset)..."

  $py = Get-Command python -ErrorAction SilentlyContinue
  if (-not $py) { throw "python not found in PATH (required for -Subset)" }

  # We intentionally don't try to pip install automatically to avoid unexpected env changes.
  & python -c "import fontTools" 2>$null
  if ($LASTEXITCODE -ne 0) {
    throw "python package 'fonttools' not found. Install it then re-run with -Subset." 
  }

  $subsetOutBold = Join-Path $OutDir "font1.subset.ttf"
  & python -m fontTools.subset $dstTtfBold --text-file=$charsetTxt --output-file=$subsetOutBold --layout-features='*' --name-IDs='*' --name-languages='*' --notdef-outline --recommended-glyphs
  if ($LASTEXITCODE -ne 0) { throw "fontTools.subset failed for font1.ttf" }

  $subsetOutRegular = Join-Path $OutDir "font2.subset.ttf"
  & python -m fontTools.subset $dstTtfRegular --text-file=$charsetTxt --output-file=$subsetOutRegular --layout-features='*' --name-IDs='*' --name-languages='*' --notdef-outline --recommended-glyphs
  if ($LASTEXITCODE -ne 0) { throw "fontTools.subset failed for font2.ttf" }

  Write-Host "[font_sync] Replace $dstTtfBold with subset output"
  Copy-Item -Force $subsetOutBold $dstTtfBold

  Write-Host "[font_sync] Replace $dstTtfRegular with subset output"
  Copy-Item -Force $subsetOutRegular $dstTtfRegular
}

Write-Host "[font_sync] Done."
