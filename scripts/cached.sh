#!/bin/bash
# Copyright 2022 The Modcncy Authors. All rights reserved.
# Use of this source code is governed by the license found in the LICENSE file.

# Some helpers.

__CYAN__=$(tput setaf 6)
__SGR0__=$(tput sgr0)

# Script.

echo $__CYAN__"-- The Modular-Concurrency Project: Current Cached Staged Files."$__SGR0__
git diff --name-only --cached
echo $__CYAN__"-- Current Cached Staged Files finished."$__SGR0__
