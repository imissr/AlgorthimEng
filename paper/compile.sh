#!/bin/bash

# LaTeX compilation script with bibtex support
# This script compiles the LaTeX document and moves all temporary files to the 'out' directory

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR" || exit 1

# Add library directory to LaTeX search path
export TEXINPUTS="$SCRIPT_DIR/library:${TEXINPUTS}"

TEXFILE="main"

# Create output directory if it doesn't exist
mkdir -p out

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}LaTeX Compilation with BibTeX${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "${YELLOW}Working directory: $SCRIPT_DIR${NC}"
echo -e "${YELLOW}Source file: $TEXFILE.tex${NC}"
echo ""

# Check if required files exist
echo -e "${BLUE}Checking dependencies...${NC}"
if [ ! -f "$TEXFILE.tex" ]; then
    echo -e "${RED}✗ Error: $TEXFILE.tex not found!${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Found $TEXFILE.tex${NC}"

if [ ! -f "library/acmart.cls" ]; then
    echo -e "${RED}✗ Error: library/acmart.cls not found!${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Found library/acmart.cls${NC}"

if [ ! -f "bibliography/sources.bib" ]; then
    echo -e "${RED}✗ Error: bibliography/sources.bib not found!${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Found bibliography/sources.bib${NC}"
echo ""

# Clean up old temporary files
echo -e "${BLUE}Cleaning up old files...${NC}"
rm -f "$TEXFILE".aux "$TEXFILE".bbl "$TEXFILE".blg "$TEXFILE".out comment.cut "$TEXFILE".log 2>/dev/null || true
echo -e "${GREEN}✓ Cleaned up${NC}"
echo ""

# Compilation steps
echo -e "${BLUE}Step 1/4: First pdflatex pass...${NC}"
pdflatex -interaction=nonstopmode "$TEXFILE.tex" > out/"$TEXFILE"_pass1.log 2>&1
if [ -f "$TEXFILE.pdf" ]; then
    echo -e "${GREEN}✓ Completed - PDF generated${NC}"
else
    echo -e "${RED}✗ Failed - No PDF generated!${NC}"
    echo -e "${RED}Log output:${NC}"
    tail -20 out/"$TEXFILE"_pass1.log
    exit 1
fi

echo -e "${BLUE}Step 2/4: Running bibtex...${NC}"
bibtex "$TEXFILE" > out/"$TEXFILE"_bibtex.log 2>&1 || true
echo -e "${GREEN}✓ Completed${NC}"

echo -e "${BLUE}Step 3/4: Second pdflatex pass...${NC}"
pdflatex -interaction=nonstopmode "$TEXFILE.tex" > out/"$TEXFILE"_pass2.log 2>&1
if [ -f "$TEXFILE.pdf" ]; then
    echo -e "${GREEN}✓ Completed - PDF updated${NC}"
else
    echo -e "${RED}✗ Failed - No PDF!${NC}"
    exit 1
fi

echo -e "${BLUE}Step 4/4: Third pdflatex pass...${NC}"
pdflatex -interaction=nonstopmode "$TEXFILE.tex" > out/"$TEXFILE"_pass3.log 2>&1
if [ -f "$TEXFILE.pdf" ]; then
    echo -e "${GREEN}✓ Completed - PDF finalized${NC}"
else
    echo -e "${RED}✗ Failed - No PDF!${NC}"
    exit 1
fi
echo ""

# Move temporary files to out directory
echo -e "${BLUE}Moving temporary files to out/...${NC}"
mv -f "$TEXFILE".aux out/ 2>/dev/null || true
mv -f "$TEXFILE".bbl out/ 2>/dev/null || true
mv -f "$TEXFILE".blg out/ 2>/dev/null || true
mv -f "$TEXFILE".out out/ 2>/dev/null || true
mv -f "$TEXFILE".log out/ 2>/dev/null || true
mv -f comment.cut out/ 2>/dev/null || true
echo -e "${GREEN}✓ Moved${NC}"
echo ""

# Verify PDF was created
echo -e "${BLUE}Verification...${NC}"
if [ -f "$TEXFILE.pdf" ]; then
    PDF_SIZE=$(du -h "$TEXFILE.pdf" | cut -f1)
    echo -e "${GREEN}✓ PDF created successfully!${NC}"
    echo -e "${GREEN}  File: $(pwd)/$TEXFILE.pdf${NC}"
    echo -e "${GREEN}  Size: $PDF_SIZE${NC}"
else
    echo -e "${RED}✗ PDF file was not created!${NC}"
    exit 1
fi

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}✓ Compilation successful!${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}Output file: $TEXFILE.pdf${NC}"
echo -e "${GREEN}Temporary files location: ./out/${NC}"
echo ""
ls -lh "$TEXFILE.pdf"
echo ""
echo -e "${BLUE}Temporary files in out/:${NC}"
ls -lh out/ | tail -n +2
