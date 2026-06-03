#!/bin/bash
set -e

REPO="ashtonjamesd/comet"
INSTALL_DIR="/usr/local/bin"
HEADER_DIR="/usr/local/include"

echo "installing comet..."

# clone to a temp dir, build, install
TMP=$(mktemp -d)
git clone --depth 1 "https://github.com/$REPO.git" "$TMP/comet"
cd "$TMP/comet"

cc src/main.c -o comet

sudo mkdir -p "$INSTALL_DIR"
sudo cp comet "$INSTALL_DIR/comet"

# install the header so users can #include <comet.h> in build.c
sudo mkdir -p "$HEADER_DIR"
sudo cp src/comet.h "$HEADER_DIR/comet.h"

# cleanup
rm -rf "$TMP"

echo "comet installed to $INSTALL_DIR/comet"
echo "header installed to /usr/local/include/comet.h"