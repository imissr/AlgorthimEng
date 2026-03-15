# LaTeX Setup Guide for Algorithm Engineering Paper

This document outlines all dependencies, libraries, and configurations needed to compile the TeX file in VS Code.

## System Dependencies

### 1. LaTeX Compiler
Install TeX Live 2023 with required packages:

```bash
# Install texlive-publishers (includes acmart.cls)
sudo apt-get install texlive-publishers

# Install texlive-extra-utils (for latexindent formatting tool)
sudo apt-get install texlive-extra-utils

# Install chktex (for LaTeX linting)
sudo apt-get install chktex

# Install cm-super (provides Type1 Computer Modern fonts for font expansion support)
sudo apt install cm-super
```

**Verification:**
```bash
which pdflatex
# Should output: /usr/bin/pdflatex
```

---

## VS Code Extensions

Install the following extensions from the VS Code Extensions Marketplace (Ctrl+Shift+X):

### Required
- **LaTeX Workshop** by James Yu (ID: `James-Yu.latex-workshop`)
  - Provides compilation, preview, and IntelliSense for LaTeX
  - Configured via `.vscode/settings.json`

### Optional (Recommended)
- **LTeX** by Valentine Wulf (ID: `valentjn.vscode-ltex`)
  - Grammar and spell-checking for LaTeX documents

---

## Project-Specific Configuration

### `.vscode/settings.json`

The project includes a `.vscode/settings.json` file with the following configuration:

```json
{
  "latex-workshop.latex.tools": [
    {
      "name": "pdflatex",
      "command": "/usr/bin/pdflatex",
      "args": [
        "-synctex=1",
        "-interaction=nonstopmode",
        "-file-line-error",
        "-output-directory=%OUTDIR%",
        "%DOC%"
      ]
    },
    {
      "name": "copy-pdf",
      "command": "bash",
      "args": [
        "-c",
        "cp %OUTDIR%/%DOCFILE%.pdf %DIR%/%DOCFILE%.pdf 2>/dev/null || true"
      ]
    }
  ],
  "latex-workshop.latex.recipes": [
    {
      "name": "pdflatex (with pdf in source)",
      "tools": ["pdflatex", "copy-pdf"]
    }
  ],
  "latex-workshop.latex.outDir": "out",
  "latex-workshop.linting.chktex.enabled": true,
  "latex-workshop.linting.chktex.args": ["-wall", "-n 21"],
  "latex-workshop.formatting.latex": "latexindent",
  "[latex]": {
    "editor.formatOnSave": true,
    "editor.defaultFormatter": "James-Yu.latex-workshop"
  }
}
```

**Key Features:**
- **Output Directory**: All auxiliary files (.log, .aux, .synctex.gz) go to `out/`
- **PDF Location**: The compiled PDF stays in the source directory with `main.tex`
- **Linting**: Enabled via chktex
- **Auto-formatting**: Enabled via latexindent on save

---

## Local Libraries

The project includes a local copy of:
- `paper/library/acmart.cls` — ACM Article Template class file

This ensures the project is self-contained and reproducible across different machines.

---

## File Structure

```
paper/
├── main.tex                 (Main LaTeX document)
├── main.pdf                 (Compiled PDF output)
├── library/
│   └── acmart.cls          (Local LaTeX class file)
├── sections/
│   ├── abstract.tex
│   ├── introduction.tex
│   ├── content.tex
│   └── appendix.tex
├── bibliography/
│   └── (bibliography files)
├── figures/
│   └── (figure images)
├── out/                     (Build artifacts - generated)
│   ├── main.log
│   ├── main.aux
│   ├── main.synctex.gz
│   └── main.pdf
└── .vscode/
    └── settings.json        (VS Code LaTeX configuration)
```

---

## Building the Document

### In VS Code
1. Open `main.tex`
2. Click the "Build LaTeX project" button in the LaTeX Workshop sidebar (or use Ctrl+Shift+B)
3. The PDF will be generated and copied to `paper/main.pdf`

### Via Terminal
```bash
cd /mnt/c/Users/aghyd/CLionProjects/Algorithm-Engineering/paper
pdflatex -synctex=1 -interaction=nonstopmode -file-line-error -output-directory=out main.tex
cp out/main.pdf main.pdf
```

---

## Formatting & Linting

### Format LaTeX File
```bash
latexindent -w paper/sections/content.tex
```

Or use VS Code's auto-format on save (enabled in settings).

### Check for LaTeX Errors
```bash
chktex paper/main.tex
```

---

## Troubleshooting

### "File `acmart.cls' not found"
- Ensure `texlive-publishers` is installed
- Or verify `paper/library/acmart.cls` exists

### "pdflatex: command not found"
- Install TeX Live: `sudo apt-get install texlive-publishers`
- Verify: `which pdflatex`

### Build output appears in source directory instead of `out/`
- Check that `.vscode/settings.json` exists with `-output-directory=%OUTDIR%` in pdflatex args

---

## Summary of Installation

```bash
# Install all system dependencies
sudo apt-get update
sudo apt-get install texlive-publishers texlive-extra-utils chktex

# Verify pdflatex is available
which pdflatex
```

Then install LaTeX Workshop and (optionally) LTeX extensions in VS Code.

The `.vscode/settings.json` is already configured in the project.
