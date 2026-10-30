#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/host
cc -O1 -w -Ifirmware/src -Itests tests/arp_shape_test.c -o build/host/arp_shape_test
build/host/arp_shape_test
for name in cr_engine_test cr_loop_test; do
    extra=""
    if [ "$name" = cr_loop_test ]; then extra="firmware/src/cr_loop.c"; fi
    cc -std=c99 -O2 -Wall -Wextra -Werror -pedantic -o "build/host/$name" "tests/$name.c" firmware/src/cr_engine.c $extra
    "build/host/$name"
done
cc -std=c11 -O2 -Wall -Wextra -Werror -pedantic -o build/host/cr_settings_test tests/cr_settings_test.c firmware/src/cr_settings.c firmware/src/cr_engine.c
build/host/cr_settings_test
cc -std=c99 -O1 -Wall -Wextra -Wno-unused-function -Werror -pedantic -fsanitize=undefined -fno-sanitize-recover=all -o build/host/cr_shape_test tests/cr_shape_test.c firmware/src/cr_engine.c firmware/src/cr_settings.c
build/host/cr_shape_test
for name in cr_backup_test cr_shape_ui_test; do
    cc -std=gnu11 -O1 -w -Itools/emu/web/compat -Ibuild/gen -Ifirmware/src -Itests -DCZ_FACTORY_RAW -o "build/host/$name" "tests/$name.c" -lm
    "build/host/$name"
done
