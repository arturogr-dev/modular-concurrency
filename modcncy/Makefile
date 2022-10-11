# The Modular-Concurrency Project.
# Copyright 2022 The Modcncy Authors. All rights reserved.
# Use of this source code is governed by the license found in the LICENSE file.

_PROJECT_ = modular_concurrency
_LIBRARY_ = modcncy
_VERSION_ = 1.0

# Some helpers.

__GREEN__   = $(shell tput setaf 2)
__YELLOW__  = $(shell tput setaf 3)
__MAGENTA__ = $(shell tput setaf 5)
__CYAN__    = $(shell tput setaf 6)
__SGR0__    = $(shell tput sgr0)

__TARGET__  = 0  # Initially zero.
__TARGETS__ = 9  # Total number of targets in Makefile, excluding `all`.
__PERCENT__ = $(shell echo "scale=0; $(__TARGET__)*100/$(__TARGETS__)" | bc)

# Build the library.

CXX_CMPLR = g++
CXX_FLAGS = -std=c++11 -Wall -Werror -Wextra -pedantic -O2

BUILD_DIR = ./build/lib
INC_PATHS = -I../ -I./include

# Add your source code file names here and as a target.
SRC_NAMES = central_sense_counter_barrier \
	central_step_counter_barrier \
	barrier \
	flags

# Add the desired output object files here.
OBJ_FILES = $(BUILD_DIR)/central_sense_counter_barrier.o \
	$(BUILD_DIR)/central_step_counter_barrier.o \
	$(BUILD_DIR)/barrier.o \
	$(BUILD_DIR)/flags.o

.PHONY: $(BUILD_DIR) \
	$(SRC_NAMES) \
	all \
	clean \
	build

all: setup clean $(BUILD_DIR) build teardown

setup:
	$(info $(__YELLOW__)-- The Modular-Concurrency Project.$(__SGR0__))
	$(eval __TARGET__=1)
	$(info $(__GREEN__)[ $(__PERCENT__)%]$(__SGR0__) $(__MAGENTA__)Start building.$(__SGR0__))

clean:
	$(eval __TARGET__=2)
	$(info $(__GREEN__)[ $(__PERCENT__)%]$(__SGR0__) Cleaning previous build version.)
	@rm -rf ./build || true

$(BUILD_DIR):
	$(eval __TARGET__=3)
	$(info $(__GREEN__)[ $(__PERCENT__)%]$(__SGR0__) Creating $(__CYAN__)$(BUILD_DIR)$(__SGR0__) directory.)
	@mkdir -p $(BUILD_DIR)

central_sense_counter_barrier: src/primitives/barriers/central_sense_counter_barrier.cc
	$(eval __TARGET__=4)
	$(info $(__GREEN__)[ $(__PERCENT__)%]$(__SGR0__) Building $(__CYAN__)$@$(__SGR0__).)
	@$(CXX_CMPLR) -c $< $(INC_PATHS) $(CXX_FLAGS) -o $(BUILD_DIR)/$@.o

central_step_counter_barrier: src/primitives/barriers/central_step_counter_barrier.cc
	$(eval __TARGET__=5)
	$(info $(__GREEN__)[ $(__PERCENT__)%]$(__SGR0__) Building $(__CYAN__)$@$(__SGR0__).)
	@$(CXX_CMPLR) -c $< $(INC_PATHS) $(CXX_FLAGS) -o $(BUILD_DIR)/$@.o

barrier: src/primitives/barriers/barrier.cc
	$(eval __TARGET__=6)
	$(info $(__GREEN__)[ $(__PERCENT__)%]$(__SGR0__) Building $(__CYAN__)$@$(__SGR0__).)
	@$(CXX_CMPLR) -c $< $(INC_PATHS) $(CXX_FLAGS) -o $(BUILD_DIR)/$@.o

flags: src/flags/flags.cc
	$(eval __TARGET__=7)
	$(info $(__GREEN__)[ $(__PERCENT__)%]$(__SGR0__) Building $(__CYAN__)$@$(__SGR0__).)
	@$(CXX_CMPLR) -c $< $(INC_PATHS) $(CXX_FLAGS) -o $(BUILD_DIR)/$@.o

build: $(SRC_NAMES)
	$(eval __TARGET__=8)
	$(info $(__GREEN__)[ $(__PERCENT__)%]$(__SGR0__) Building $(__CYAN__)$(BUILD_DIR)/lib$(_LIBRARY_).a$(__SGR0__).)
	@ar -crs $(BUILD_DIR)/lib$(_LIBRARY_).a $(OBJ_FILES)
	
teardown:
	$(eval __TARGET__=9)
	$(info $(__GREEN__)[$(__PERCENT__)%]$(__SGR0__) $(__MAGENTA__)Finished successfully.$(__SGR0__))
