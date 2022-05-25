#!/bin/bash
# Copyright 2022 The Modcncy Authors. All rights reserved.
# Use of this source code is governed by the license found in the LICENSE file.

# Some helpers.

__CYAN__=$(tput setaf 6)
__SGR0__=$(tput sgr0)

# Script.

echo $__CYAN__"-- The Modular-Concurrency Project: Working-Tree Untracked Files."$__SGR0__
git ls-files --others --exclude-standard
echo $__CYAN__"-- Working-Tree Untracked Files finished."$__SGR0__
