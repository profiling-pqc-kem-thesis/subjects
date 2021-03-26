#!/usr/bin/env bash

CFLAGS="-Wstack-usage=1 -Werror=stack-usage=1" make --always-make --directory "$@" 2>&1 | grep -e '-Werror=stack-usage=' -B1 | grep -o -e "In function .[a-zA-Z_0-9]\+.\|stack usage \(is\|might be\) \([0-9]\+ bytes\|unbounded\)" | sed -e "s/In function .\([a-zA-Z_0-9]\+\).\|stack usage \(is\|might be\) \([0-9]\+ bytes\|unbounded\)/\1\3/g" | paste -sd ' \n'
