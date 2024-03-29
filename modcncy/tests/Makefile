# The Modular-Concurrency Project Test Cases.
# Copyright 2022 The Modcncy Authors. All rights reserved.
# Use of this source code is governed by the license found in the LICENSE file.

_PROJECT_ = modular_concurrency_tests
_LIBRARY_ = modcncy_tests
_VERSION_ = 1.0.tests

# Some helpers.

__MAGENTA__ = $(shell tput setaf 5)
__CYAN__    = $(shell tput setaf 6)
__SGR0__    = $(shell tput sgr0)

# Build tests.

CXX_CMPLR = g++
CXX_FLAGS = -std=c++11 -Wall -Werror -Wextra -pedantic -O2 -march=native -mtune=native -flto -fuse-linker-plugin

MODCNCY_LIB_DIR = ..
THIRD_PARTY_DIR = ../../third_party

BUILD_DIR = ./build
INC_PATHS = -I$(MODCNCY_LIB_DIR)/include -I$(THIRD_PARTY_DIR)/googletest/googletest/include
LNK_PATHS = -L$(MODCNCY_LIB_DIR)/build/lib -L$(THIRD_PARTY_DIR)/googletest/build/lib
LNK_NAMES = -lmodcncy -lgtest -lgtest_main -lpthread

# Use `flakiness` as an input from user to repeat the test multiple times.
flakiness = no
ifneq ($(flakiness), no)
	gtest_flags = --gtest_repeat=100 --gtest_break_on_failure
endif

# Add your tests here with the prefix `run_` and as a target.
TESTS = run_barrier_test \
	run_flags_test \
	run_concurrent_task_queue_test

.PHONY: all \
	setup \
	clean \
	$(BUILD_DIR) \
	test \
	$(TESTS) \
	teardown

all: setup clean $(BUILD_DIR) test teardown

setup:
	$(info $(__MAGENTA__)-- The Modular-Concurrency Project: Unit Testing.$(__SGR0__))

clean:
	$(info $(__CYAN__)-- Cleaning previous build tests.$(__SGR0__))
	@rm -rf ./build || true

$(BUILD_DIR):
	$(info $(__CYAN__)-- Creating build tests directory.$(__SGR0__))
	@mkdir -p $(BUILD_DIR)

barrier_test: barrier_test.cc
	$(info $(__CYAN__)-- Compiling $<$(__SGR0__))
	$(CXX_CMPLR) $< $(INC_PATHS) $(LNK_PATHS) $(LNK_NAMES) $(CXX_FLAGS) -o $(BUILD_DIR)/run_$@
run_barrier_test: barrier_test
	$(info $(__CYAN__)-- Running $@$(__SGR0__))
	$(BUILD_DIR)/$@ $(gtest_flags)

flags_test: flags_test.cc
	$(info $(__CYAN__)-- Compiling $<$(__SGR0__))
	$(CXX_CMPLR) $< $(INC_PATHS) $(LNK_PATHS) $(LNK_NAMES) $(CXX_FLAGS) -o $(BUILD_DIR)/run_$@
run_flags_test: flags_test
	$(info $(__CYAN__)-- Running $@$(__SGR0__))
	$(BUILD_DIR)/$@ $(gtest_flags)

concurrent_task_queue_test: concurrent_task_queue_test.cc
	$(info $(__CYAN__)-- Compiling $<$(__SGR0__))
	$(CXX_CMPLR) $< $(INC_PATHS) $(LNK_PATHS) $(LNK_NAMES) $(CXX_FLAGS) -o $(BUILD_DIR)/run_$@
run_concurrent_task_queue_test: concurrent_task_queue_test
	$(info $(__CYAN__)-- Running $@$(__SGR0__))
	$(BUILD_DIR)/$@ $(gtest_flags)

test: $(TESTS)
	
teardown:
	$(info $(__MAGENTA__)-- Tests finished.$(__SGR0__))
