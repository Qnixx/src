#!/usr/bin/env bash

# find the location of the source code
srcdir="$(dirname "$0")"
test -z "$srcdir" && srcdir=.
cd "$srcdir"

# get system "install-sh" and run autoconf
cp "$(automake --print-libdir)/install-sh" . && autoconf
