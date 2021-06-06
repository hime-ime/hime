#!/usr/bin/env bash

set -euo pipefail
set -x

separator() {
    echo -e "\n\n\n\n\n"
}

# base
apt-get update --yes && apt-get upgrade --yes
apt-get install --yes \
        build-essential \
        locales \
        fakeroot \
        devscripts \
        git

# dependencies
apt-get install --yes \
        libxtst-dev \
        libgtk2.0-dev \
        libgtk-3-dev \
        libanthy-dev \
        libchewing3-dev \
        libappindicator-dev \
        libappindicator3-dev \
        qtbase5-private-dev

echo 'en_US.UTF-8 UTF-8' >/etc/locale.gen && locale-gen

set +x; separator; separator; separator; set -x

# enter GitHub workspace directory path
cd "$GITHUB_WORKSPACE"

# build Debian package
./distro/debian/gen-deb
