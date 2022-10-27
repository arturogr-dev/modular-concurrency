// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <modcncy/flags.h>

#include <cstdlib>

// =============================================================================
TEST(Int32FromEnvTest, NotInEnv) {
  ASSERT_EQ(unsetenv("NOT_IN_ENV"), 0);
  EXPECT_EQ(modcncy::Int32FromEnv("not_in_env", 42), 42);
}

// =============================================================================
TEST(Int32FromEnvTest, InvalidInteger) {
  ASSERT_EQ(setenv("IN_ENV", "foo", 1), 0);
  EXPECT_EQ(modcncy::Int32FromEnv("in_env", 42), 42);
  unsetenv("IN_ENV");
}

// =============================================================================
TEST(Int32FromEnvTest, IntegerOverflow) {
  ASSERT_EQ(setenv("IN_ENV", "2147483648", 1), 0);  // LONG_MAX + 1
  EXPECT_EQ(modcncy::Int32FromEnv("in_env", 42), 42);
  unsetenv("IN_ENV");
}

// =============================================================================
TEST(Int32FromEnvTest, ValidInteger) {
  ASSERT_EQ(setenv("IN_ENV", "42", 1), 0);
  EXPECT_EQ(modcncy::Int32FromEnv("in_env", 64), 42);
  unsetenv("IN_ENV");
}

// =============================================================================
TEST(Int32FromEnvTest, MaxValidInteger) {
  ASSERT_EQ(setenv("IN_ENV", "2147483647", 1), 0);  // LONG_MAX
  EXPECT_EQ(modcncy::Int32FromEnv("in_env", 64), 2147483647);
  unsetenv("IN_ENV");
}
