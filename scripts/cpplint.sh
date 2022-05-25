#!/bin/bash
# Copyright 2022 The Modcncy Authors. All rights reserved.
# Use of this source code is governed by the license found in the LICENSE file.

# Some helpers.

__CYAN__=$(tput setaf 6)
__SGR0__=$(tput sgr0)

# Script.

echo $__CYAN__"-- The Modular-Concurrency Project: Cpplint Static Code Checker."$__SGR0__
cpplint $(find -not \( -path ./third_party -prune \) \( -name "*.h" -o -name "*.cc" \))
echo $__CYAN__"-- Cpplint Static Code Checker finished."$__SGR0__
