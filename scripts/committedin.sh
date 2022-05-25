#!/bin/bash
# Copyright 2022 The Modcncy Authors. All rights reserved.
# Use of this source code is governed by the license found in the LICENSE file.

# Some helpers.

__RED__=$(tput setaf 1)
__MAGENTA__=$(tput setaf 5)
__CYAN__=$(tput setaf 6)
__SGR0__=$(tput sgr0)

# Script.

if [ -z "$1" ]
then
  this_commit=HEAD
else
  this_commit=$1
fi

echo $__CYAN__"-- The Modular-Concurrency Project: Files Committed in$__SGR0__ $__RED__$this_commit$__SGR0__"
echo $__MAGENTA__"-- The script takes the SHA as an input (HEAD if not specified)."$__SGR0__
git show --pretty="" --name-only $this_commit
echo $__CYAN__"-- Files Committed in$__SGR0__ $__RED__$this_commit$__SGR0__$__CYAN__ finished.$__SGR0__"
