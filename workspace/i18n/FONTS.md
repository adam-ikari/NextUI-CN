# Fonts (downstream overlay)

This repo is a **downstream localization fork**. To keep rebases painless, we treat fonts as an **overlay asset** (no upstream code changes required).

## Where fonts live

NextUI loads UI fonts from `/.system/res/` (see `RES_PATH` and `CFG_setFontId()` in `workspace/all/common/config.c`).

In this repo we supply those files via the skeleton overlay:

- `skeleton/SYSTEM/res/font1.ttf`
- `skeleton/SYSTEM/res/font2.ttf`

Anything under `skeleton/` is intended to be copied into the final SD card layout, so it survives upstream rebases.

## Dream Han Sans CN mapping (current convention)

We use Dream Han Sans CN from:

`https://github.com/Pal3love/dream-han-cjk/releases`

And map release files into NextUI assets:

- `DreamHanSansCN-Bold.ttf` → `skeleton/SYSTEM/res/BPreplayBold-unhinted.ttf`
- `DreamHanSansCN-Bold.ttf` → `skeleton/SYSTEM/res/BPreplayBold.ttf` (alias)
- `DreamHanSansCN-Bold.ttf` → `skeleton/SYSTEM/res/font1.ttf` (默认字体，粗体字重)
- `DreamHanSansCN-Regular.ttf` → `skeleton/SYSTEM/res/font2.ttf` (可选字体，常规字重)

Notes:
- `BPreplayBold-unhinted.ttf` is used by some UI elements as a bold face.
- `font1.ttf` is the default UI font (ID depends on upstream config).
- We use the DreamHanSansCN variant (简体中文版本) for optimal Chinese character support.
- The CN version is specifically optimized for Simplified Chinese characters.
- Dream Han Sans CN provides 27 weights and is based on Source Han Sans 2.004.

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
