#!/bin/bash
set -e

REPO="ashtonjamesd/c-init"
INSTALL_DIR="/usr/local/bin"
HEADER_DIR="$HOME/.cinit/include"

echo "installing cinit..."

# clone to a temp dir, build, install
TMP=$(mktemp -d)
git clone --depth 1 "https://github.com/$REPO.git" "$TMP/c-init"
cd "$TMP/c-init"

cc src/main.c -o cinit

sudo mkdir -p "$INSTALL_DIR"
sudo cp cinit "$INSTALL_DIR/cinit"

# install the header so users can #include <cinit.h> in build.c
mkdir -p "$HEADER_DIR"
sudo mkdir -p /usr/local/include
sudo cp src/cinit.h /usr/local/include/cinit.h

# cleanup
rm -rf "$TMP"

echo "cinit installed to $INSTALL_DIR/cinit"
echo "header installed to $HEADER_DIR/cinit.h"