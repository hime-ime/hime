#!/usr/bin/env bash

set -euo pipefail
set -x

separator() {
    echo -e "\n\n\n\n\n"
}

format_check() {
    make clang-format
    git --no-pager diff --exit-code
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
    qt5-base

echo 'en_US.UTF-8 UTF-8' >/etc/locale.gen && locale-gen

set +x; separator; separator; separator; set -x

# enter GitHub workspace directory path
cd "$GITHUB_WORKSPACE"

# check code formatting
format_check

# configure and build
./configure --prefix=/usr --qt5-im-module-path=/usr/lib/qt/plugins/platforminputcontexts/
make -j "$(nproc)"
