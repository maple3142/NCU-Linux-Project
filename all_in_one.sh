#!/bin/sh
time env KBUILD_BUILD_TIMESTAMP='' make -j6 CC="ccache gcc"; and sudo make modules_install; and sudo make install; and sudo update-grub

