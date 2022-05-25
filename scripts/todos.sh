#!/bin/bash
# Copyright 2022 The Modcncy Authors. All rights reserved.
# Use of this source code is governed by the license found in the LICENSE file.

# Some helpers.

__CYAN__=$(tput setaf 6)
__SGR0__=$(tput sgr0)

# Script.

echo $__CYAN__"-- The Modular-Concurrency Project: TODOs."$__SGR0__
grep --color=always --exclude-dir=third_party/ --exclude=todos.sh -r "TODO"
echo $__CYAN__"-- TODOs finished."$__SGR0__
