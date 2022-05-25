#!/bin/bash
# Copyright 2022 The Modcncy Authors. All rights reserved.
# Use of this source code is governed by the license found in the LICENSE file.

# Some helpers.

__RED__=$(tput setaf 1)
__GREEN__=$(tput setaf 2)
__YELLOW__=$(tput setaf 3)
__BLUE__=$(tput setaf 4)
__MAGENTA__=$(tput setaf 5)
__CYAN__=$(tput setaf 6)
__SGR0__=$(tput sgr0)

# Script.

this_branch=$(git branch --show-current)
this_commit=HEAD

echo $__YELLOW__"-- The Modular-Concurrency Project: Repository Statistics."$__SGR0__
echo "-- The script produces a summary of lines of code of the current repository."
echo $__CYAN__"-- Start of report."$__SGR0__
echo $__GREEN__"-- On branch $__BLUE__$this_branch$__SGR0__"$__SGR0__

echo $__GREEN__"-- AT COMMIT $__BLUE__$this_commit$__SGR0__"$__SGR0__
if [[ $(git ls-files -- . ':!:*third_party*' | wc -l) -ne 0 ]]; then
  echo $__MAGENTA__"---------------------------------"$__SGR0__
  echo $__MAGENTA__"   LINES OF CODE     TYPE OF FILE"$__SGR0__
  echo $__MAGENTA__"---------------------------------"$__SGR0__
  wc -l $(git ls-files -- . ':!:*third_party*') | \
    awk -F'[\ /\.]+' '/\.|Makefile/ { if ( $1 ) { sumlines[$NF] += $1 } else { sumlines[$NF] += $2 } } END { for (ext in sumlines) { printf "%16d %16s\n", sumlines[ext], ext } }' | sort -nr
else
  echo $__RED__"-- No files found in the repository."$__SGR0__
fi

echo $__CYAN__"-- End of report."$__SGR0__
