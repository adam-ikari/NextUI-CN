# Fonts (downstream overlay)

This repo is a **downstream localization fork**. To keep rebases painless, we treat fonts as an **overlay asset** (no upstream code changes required).

## Where fonts live

NextUI loads UI fonts from `/.system/res/` (see `RES_PATH` and `CFG_setFontId()` in `workspace/all/common/config.c`).

In this repo we supply those files via the skeleton overlay:

- `skeleton/SYSTEM/res/font1.ttf`
- `skeleton/SYSTEM/res/font2.ttf`

Anything under `skeleton/` is intended to be copied into the final SD card layout, so it survives upstream rebases.

## ChillRound mapping (current convention)

We use ChillRound from:

`https://github.com/Warren2060/ChillRound/releases`

And map release files into NextUI assets:

- `ChillRoundM.otf`  → `skeleton/SYSTEM/res/BPreplayBold-unhinted.otf`
- `ChillRoundM.otf`  → `skeleton/SYSTEM/res/BPreplayBold.otf` (alias)
- `ChillRound.ttf` (or `ChillRoundM.ttf` in some releases) → `skeleton/SYSTEM/res/font1.ttf`

Notes:
- `BPreplayBold-unhinted.otf` is used by some UI elements as a bold face.
- `font1.ttf` is the default UI font (ID depends on upstream config).

## Re-sync fonts (Windows PowerShell)

From repo root:

```powershell
pwsh -File workspace/i18n/font_sync.ps1
```

Force re-download:

```powershell
pwsh -File workspace/i18n/font_sync.ps1 -Force
```

### Optional: subset to reduce size

If you have Python + `fonttools` installed, you can subset `font1.ttf` using the characters found in `workspace/i18n/locales/zh_CN.lang`:

```powershell
pwsh -File workspace/i18n/font_sync.ps1 -Subset
```

This keeps the UI package smaller, and still covers all translated strings.
