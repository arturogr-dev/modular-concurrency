# The Modular-Concurrency Project Benchmarks.
# Copyright 2022 The Modcncy Authors. All rights reserved.
# Use of this source code is governed by the license found in the LICENSE file.

_PROJECT_ = modular_concurrency_benchmarks
_LIBRARY_ = modcncy_benchmarks
_VERSION_ = 1.0.benchmarks

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
INC_PATHS = -I$(MODCNCY_LIB_DIR)/include -I$(THIRD_PARTY_DIR)/benchmark/include -I$(THIRD_PARTY_DIR)/benchmark/build/include
LNK_PATHS = -L$(MODCNCY_LIB_DIR)/build/lib -L$(THIRD_PARTY_DIR)/benchmark/build/src
LNK_NAMES = -lmodcncy -lbenchmark -lpthread

# Use `repetitions` as an input from user to repeat the benchmark multiple times.
# Usually use it with some benchmark filter (use `BENCH_ARGS`) to avoid an unnecessary big report.
repetitions = no
benchmark_flags = --benchmark_counters_tabular=true
ifneq ($(repetitions), no)
	benchmark_flags += --benchmark_enable_random_interleaving=true --benchmark_repetitions=10 --benchmark_min_time=0.1
endif

# Use `benchmark_args` as an input for additional benchmark flags. For example:
# $ make benchmark_args=--benchmark_filter=<some_regex>

# Add your benchmarks here with the prefix `run_` and as a target.
BENCHMARKS = run_barrier_benchmark

.PHONY: all \
	setup \
	clean \
	$(BUILD_DIR) \
	benchmark \
	$(BENCHMARKS) \
	teardown

all: setup clean $(BUILD_DIR) benchmark teardown

setup:
	$(info $(__MAGENTA__)-- The Modular-Concurrency Project: Benchmarking.$(__SGR0__))

clean:
	$(info $(__CYAN__)-- Cleaning previous build benchmarks.$(__SGR0__))
	@rm -rf ./build || true

$(BUILD_DIR):
	$(info $(__CYAN__)-- Creating build benchmark directory.$(__SGR0__))
	@mkdir -p $(BUILD_DIR)

barrier_benchmark: barrier_benchmark.cc
	$(info $(__CYAN__)-- Compiling $<$(__SGR0__))
	$(CXX_CMPLR) $< $(INC_PATHS) $(LNK_PATHS) $(LNK_NAMES) $(CXX_FLAGS) -o $(BUILD_DIR)/run_$@
run_barrier_benchmark: barrier_benchmark
	$(info $(__CYAN__)-- Running $@$(__SGR0__))
	$(BUILD_DIR)/$@ $(benchmark_flags) $(benchmark_args)

benchmark: $(BENCHMARKS)
	
teardown:
	$(info $(__MAGENTA__)-- Benchmarks finished.$(__SGR0__))
