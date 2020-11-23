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
    qt5-qtbase-devel \
    qt5-qtbase-private-devel \


set +x; separator; separator; separator; set -x

# enter GitHub workspace directory path
cd "$GITHUB_WORKSPACE"

./configure --disable-gtk2-im-module --disable-gtk3-im-module \
    --prefix="$PWD/build" \
    --qt5-im-module-path=/usr/lib/qt/plugins/platforminputcontexts/

make
