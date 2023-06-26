#!/usr/bin/env bash

set -euo pipefail
set -x

separator() {
    echo -e "\n\n\n\n\n"
}

# base
yum update -y
yum groupinstall -y 'Development Tools'

# dependencies
yum install -y \
    desktop-file-utils \
    gettext \
    gtk2-devel\
    gtk3-devel \
    libXtst-devel \
    libchewing-devel \
    qt5-qtbase-devel


set +x; separator; separator; separator; set -x

# enter GitHub workspace directory path
cd "$GITHUB_WORKSPACE"

export CFLAGS="-std=gnu11"
export CXXFLAGS="-std=gnu++11"

autoreconf -i
./configure \
    --disable-qt6-immodule \
    --prefix="$PWD/build" \
    --with-qt5-im-module-path=/usr/lib/qt/plugins/platforminputcontexts/

make
