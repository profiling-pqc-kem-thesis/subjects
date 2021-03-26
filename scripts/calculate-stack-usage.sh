#!/usr/bin/env bash

export CFLAGS="-Wstack-usage=1 -Werror=stack-usage=1"

make --always-make --directory "$@" 2>&1 | grep -e '-Werror=stack-usage=' -B1 | grep -o -e "In function '[^']\+'\|stack usage \(is\|might be\) [0-9]\+ bytes" | sed -e "s/In function '\([^']\+\)'\|stack usage \(is\|might be\) \([0-9]\+\) bytes/\1\3/g" | paste -sd ' \n'
