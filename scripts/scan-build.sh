#!/usr/bin/env bash

# Run Clang's static analyzer scan-build
#
# This script will build the hime project with scan-build and open the report
# in the browser: http://127.0.0.1:8181
#
# https://clang-analyzer.llvm.org/scan-build.html

set -ueo pipefail

make distclean

scan-build ./configure --prefix=/usr --disable-lib64 --qt5-im-module-path=/usr/lib/qt/plugins/platforminputcontexts/
scan-build -V make -j "$(nproc)"
