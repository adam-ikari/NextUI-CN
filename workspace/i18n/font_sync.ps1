<#
.SYNOPSIS
  Sync Dream Han Sans CN fonts into this downstream repo in a reproducible way.

.DESCRIPTION
  Downloads the Dream Han Sans CN release ZIP, extracts the two files you use and
  copies them into skeleton/SYSTEM/res with the filenames expected by NextUI.

  Mapping (as per your current convention):
    DreamHanSansCN-Bold.ttf -> skeleton/SYSTEM/res/BPreplayBold-unhinted.ttf
    DreamHanSansCN-Bold.ttf -> skeleton/SYSTEM/res/BPreplayBold.ttf
    DreamHanSansCN-Bold.ttf -> skeleton/SYSTEM/res/font1.ttf
    DreamHanSansCN-Regular.ttf -> skeleton/SYSTEM/res/font2.ttf

  Optional subset:
    If you set -Subset and have python + fonttools available, this script will
    subset DreamHanSansCN-Regular.ttf using characters found in: workspace/i18n/locales/zh_CN.lang

.NOTES
  This script is written for Windows PowerShell.
#>

param(
  [string]$Url = "https://github.com/Pal3love/dream-han-cjk/releases/download/dream-3.02-sans-2.004-serif-2.003/DreamHanSansCN.zip",
  [string]$OutDir = "$PSScriptRoot/.cache/fonts",
  [switch]$Force,
  [switch]$Subset
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$zipPath = Join-Path $OutDir "DreamHanSansCN.zip"
$tmpExtract = Join-Path $OutDir "extract"

$dstRes = Join-Path $repoRoot "skeleton\SYSTEM\res"
$dstTtfBold = Join-Path $dstRes "BPreplayBold-unhinted.ttf"
$dstTtfBoldAlias = Join-Path $dstRes "BPreplayBold.ttf"
$dstTtf1 = Join-Path $dstRes "font1.ttf"
$dstTtf2 = Join-Path $dstRes "font2.ttf"

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
$srcTtfBold = Get-ChildItem -Path $tmpExtract -Recurse -File -Filter "*Bold*.ttf" | Select-Object -First 1
$srcTtfRegular = Get-ChildItem -Path $tmpExtract -Recurse -File -Filter "*Regular*.ttf" | Select-Object -First 1

if (-not $srcTtfBold -or -not $srcTtfRegular) { throw "Dream Han Sans CN TTF fonts not found inside zip" }

New-Item -ItemType Directory -Force -Path $dstRes | Out-Null

Write-Host "[font_sync] Copy: $($srcTtfBold.FullName) -> $dstTtfBold"
Copy-Item -Force $srcTtfBold.FullName $dstTtfBold

Write-Host "[font_sync] Copy: $($srcTtfBold.FullName) -> $dstTtfBoldAlias"
Copy-Item -Force $srcTtfBold.FullName $dstTtfBoldAlias

Write-Host "[font_sync] Copy (bold): $($srcTtfBold.FullName) -> $dstTtf1"
Copy-Item -Force $srcTtfBold.FullName $dstTtf1

Write-Host "[font_sync] Copy (regular): $($srcTtfRegular.FullName) -> $dstTtf2"
Copy-Item -Force $srcTtfRegular.FullName $dstTtf2

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
  & python -m fontTools.subset $dstTtf1 --text-file=$charsetTxt --output-file=$subsetOutBold --layout-features='*' --name-IDs='*' --name-languages='*' --notdef-outline --recommended-glyphs
  if ($LASTEXITCODE -ne 0) { throw "fontTools.subset failed for font1.ttf" }

  $subsetOutRegular = Join-Path $OutDir "font2.subset.ttf"
  & python -m fontTools.subset $dstTtf2 --text-file=$charsetTxt --output-file=$subsetOutRegular --layout-features='*' --name-IDs='*' --name-languages='*' --notdef-outline --recommended-glyphs
  if ($LASTEXITCODE -ne 0) { throw "fontTools.subset failed for font2.ttf" }

  Write-Host "[font_sync] Replace $dstTtf1 with subset output"
  Copy-Item -Force $subsetOutBold $dstTtf1

  Write-Host "[font_sync] Replace $dstTtf2 with subset output"
  Copy-Item -Force $subsetOutRegular $dstTtf2
}

Write-Host "[font_sync] Done."
