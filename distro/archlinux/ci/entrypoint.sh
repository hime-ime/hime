#!/usr/bin/env bash

set -euo pipefail
set -x

separator() {
    echo -e "\n\n\n\n\n"
}

# base
pacman -Syuu --noconfirm --needed \
    base \
    base-devel \
    git

# dependencies
pacman -Syuu --noconfirm --needed \
    clang \
    libxtst \
    gtk2 \
    gtk3 \
    anthy \
    libchewing \
    libappindicator-gtk2 \
    libappindicator-gtk3 \
    qt5-base \
    qt6-base

echo 'en_US.UTF-8 UTF-8' >/etc/locale.gen && locale-gen

set +x; separator; separator; separator; set -x

# enter GitHub workspace directory path
cd "$GITHUB_WORKSPACE"
git config --global --add safe.directory "$GITHUB_WORKSPACE"

# configure and build
./configure --prefix=/usr --with-qt5-im-module-path=/usr/lib/qt/plugins/platforminputcontexts/
make -j "$(nproc)"
