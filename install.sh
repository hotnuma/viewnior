#!/usr/bin/bash

BASEDIR="$(dirname -- "$(readlink -f -- "$0";)")"

NOCLEAN=0
BUILDTYPE="plain"

# libgdk-pixbuf-2.0-dev libexiv2-dev

while [[ $# > 0 ]]; do
    key="$1"
    case $key in
        noclean)
        NOCLEAN=1
        shift
        ;;
        debug)
        BUILDTYPE="debug"
        shift
        ;;
        *)
        shift
        ;;
    esac
done

dest=$BASEDIR/build
if [[ $NOCLEAN == 0 && -d $dest ]]; then
    rm -rf $dest
fi

meson setup build -Dbuildtype=${BUILDTYPE}
ninja -C build
sudo ninja -C build install


