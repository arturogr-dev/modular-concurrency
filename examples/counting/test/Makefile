# The Modular-Concurrency Project Examples, matrix multiplication.
# Copyright 2022 The Modcncy Authors. All rights reserved.
# Use of this source code is governed by the license found in the LICENSE file.

_PROJECT_ = modular_concurrency_examples_counting
_LIBRARY_ = modcncy_examples
_VERSION_ = 1.0.examples.counting

# Some helpers.

__MAGENTA__ = $(shell tput setaf 5)
__CYAN__    = $(shell tput setaf 6)
__SGR0__    = $(shell tput sgr0)

# Build tests.

CXX_CMPLR = g++
CXX_FLAGS = -std=c++11 -Wall -Werror -Wextra -pedantic -O2 -fopenmp -march=native -mtune=native -flto -fuse-linker-plugin
BUILD_DIR = ./build

ATOMICCOUNT_DIR = ../../..
MODCNCY_LIB_DIR = ../../../modcncy
THIRD_PARTY_DIR = ../../../third_party

INC_PATHS_TEST = -I$(ATOMICCOUNT_DIR) -I$(MODCNCY_LIB_DIR)/include -I$(THIRD_PARTY_DIR)/googletest/googletest/include
LNK_PATHS_TEST = -L$(MODCNCY_LIB_DIR)/build/lib -L$(THIRD_PARTY_DIR)/googletest/build/lib
LNK_NAMES_TEST = -lmodcncy -lgtest -lgtest_main -lpthread

# Use `flakiness` as an input from user to repeat the test multiple times.
flakiness = no
ifneq ($(flakiness), no)
  gtest_flags = --gtest_repeat=100 --gtest_break_on_failure
endif

INC_PATHS_BENCH = -I$(ATOMICCOUNT_DIR) -I$(MODCNCY_LIB_DIR)/include -I$(THIRD_PARTY_DIR)/benchmark/include -I$(THIRD_PARTY_DIR)/benchmark/build/include
LNK_PATHS_BENCH = -L$(MODCNCY_LIB_DIR)/build/lib -L$(THIRD_PARTY_DIR)/benchmark/build/src
LNK_NAMES_BENCH = -lmodcncy -lbenchmark -lpthread

# Use `repetitions` as an input from user to repeat the benchmark multiple times.
# Usually use it with some benchmark filter (use `benchmark_args`) to avoid an unnecessary big report.
repetitions = no
benchmark_flags = --benchmark_counters_tabular=true
ifneq ($(repetitions), no)
  benchmark_flags += --benchmark_enable_random_interleaving=true --benchmark_repetitions=10 --benchmark_min_time=0.1
endif

.PHONY: all \
	setup \
	clean \
	$(BUILD_DIR) \
	test \
	benchmark \
	teardown

all: setup clean $(BUILD_DIR) run_counting_test run_counting_benchmark teardown
test: setup clean $(BUILD_DIR) run_counting_test teardown
benchmark: setup clean $(BUILD_DIR) run_counting_benchmark teardown

setup:
	$(info $(__MAGENTA__)-- The Modular-Concurrency Project: Examples - Counting.$(__SGR0__))

clean:
	$(info $(__CYAN__)-- Cleaning previous counting build.$(__SGR0__))
	@rm -rf ./build || true

$(BUILD_DIR):
	$(info $(__CYAN__)-- Creating counting build directory.$(__SGR0__))
	@mkdir -p $(BUILD_DIR)

counting_test: counting_test.cc
	$(info $(__CYAN__)-- Compiling $<$(__SGR0__))
	$(CXX_CMPLR) $< algorithm.cc $(INC_PATHS_TEST) $(LNK_PATHS_TEST) $(LNK_NAMES_TEST) $(CXX_FLAGS) -o $(BUILD_DIR)/run_$@
run_counting_test: counting_test
	$(info $(__CYAN__)-- Running $@$(__SGR0__))
	$(BUILD_DIR)/$@ $(gtest_flags)

counting_benchmark: counting_benchmark.cc
	$(info $(__CYAN__)-- Compiling $<$(__SGR0__))
	$(CXX_CMPLR) $< algorithm.cc $(INC_PATHS_BENCH) $(LNK_PATHS_BENCH) $(LNK_NAMES_BENCH) $(CXX_FLAGS) -o $(BUILD_DIR)/run_$@
run_counting_benchmark: counting_benchmark
	$(info $(__CYAN__)-- Running $@$(__SGR0__))
	$(BUILD_DIR)/$@ $(benchmark_flags) $(benchmark_args)

teardown:
	$(info $(__MAGENTA__)-- Counting finished.$(__SGR0__))
