// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <modcncy/flags.h>

#include <cstdlib>

// =============================================================================
TEST(IntFromEnvTest, NotInEnv) {
  ASSERT_EQ(unsetenv("NOT_IN_ENV"), 0);
  EXPECT_EQ(modcncy::IntFromEnv("not_in_env", 42), 42);
}

// =============================================================================
TEST(IntFromEnvTest, InvalidInteger) {
  ASSERT_EQ(setenv("IN_ENV", "foo", 1), 0);
  EXPECT_EQ(modcncy::IntFromEnv("in_env", 42), 42);
  unsetenv("IN_ENV");
}

// =============================================================================
TEST(IntFromEnvTest, IntegerOverflow) {
  ASSERT_EQ(setenv("IN_ENV", "2147483648", 1), 0);  // LONG_MAX + 1
  EXPECT_EQ(modcncy::IntFromEnv("in_env", 42), 42);
  unsetenv("IN_ENV");
}

// =============================================================================
TEST(IntFromEnvTest, ValidInteger) {
  ASSERT_EQ(setenv("IN_ENV", "42", 1), 0);
  EXPECT_EQ(modcncy::IntFromEnv("in_env", 64), 42);
  unsetenv("IN_ENV");
}

// =============================================================================
TEST(IntFromEnvTest, MaxValidInteger) {
  ASSERT_EQ(setenv("IN_ENV", "2147483647", 1), 0);  // LONG_MAX
  EXPECT_EQ(modcncy::IntFromEnv("in_env", 64), 2147483647);
  unsetenv("IN_ENV");
}
