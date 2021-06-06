#!/usr/bin/env bash

set -euo pipefail

topleveldir="$(git rev-parse --show-toplevel)"
archdir="$topleveldir/distro/archlinux"

pushd "$archdir" || exit 1
set -x

# clean up
rm -rf hime/ pkg/ src/

makepkg -sf

ls -ltr ./*.pkg.tar.*

set +x
popd || exit 1
