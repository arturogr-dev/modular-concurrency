// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------

#include "modcncy/include/modcncy/flags.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>

namespace modcncy {

// =============================================================================
// Returns the name of the environment variable corresponding to the given flag.
// For example, `FlagToEnvVar("foo")` will return `MODCNCY_FOO`.
static std::string FlagToEnvVar(const char* flag) {
  const std::string flag_str(flag);
  std::string env_var;
  for (size_t i = 0; i < flag_str.length(); ++i)
    env_var += static_cast<char>(toupper(flag_str.c_str()[i]));
  return env_var;
}

// =============================================================================
// Parses a `string` as a command line flag.
// The string should have the format "--flag=value".
// Returns the value of the flag or `nullptr` if the parsing fails.
const char* ParseFlagValue(const char* str, const char* flag) {
  // `str` and `flag` must not be nullptr.
  if (str == nullptr || flag == nullptr) return nullptr;
  // The flag must start with "--".
  const std::string flag_str = std::string("--") + std::string(flag);
  const size_t flag_len = flag_str.length();
  if (strncmp(str, flag_str.c_str(), flag_len) != 0) return nullptr;
  // Skip the flag name.
  const char* flag_end = str + flag_len;
  // There must be a "=" after the flag name.
  if (flag_end[0] != '=') return nullptr;
  // Return the string after "=".
  return flag_end + 1;
}

// =============================================================================
// Parses `str` to an `int` data type.
// If successful, writes the result to `*value` and returns true.
// Otherwise, leaves `*value` unchanged and returns false.
bool ParseInt(const std::string& src_text, const char* str, int* value) {
  // Parse the environment variable as a decimal integer.
  char* end = nullptr;
  const long long_value = strtol(str, &end, 10);  // NOLINT(runtime/int)
  // Has `strtol()` consumed all characters in the string?
  if (*end != '\0') {
    // No, an invalid character was encountered.
    std::cerr << src_text << " is expected to be an integer, but has value \""
              << str << "\", which is invalid.\n";
    return false;
  }
  // Is the parsed value in the range of an `int`?
  const int result = static_cast<int>(long_value);
  if (long_value == std::numeric_limits<long>::max() ||  // NOLINT(runtime/int)
      long_value == std::numeric_limits<long>::min() ||  // NOLINT(runtime/int)
      result != long_value) {
    // The parsed value overflows as a `long` or as an `int`.
    // `strtol()` returns LONG_MAX or LONG_MIN when the input overflows.
    std::cerr << src_text << " is expected to be an integer, but has value \""
              << str << "\", which overflows.\n";
    return false;
  }
  *value = result;
  return true;
}

// =============================================================================
int IntFromEnv(const char* flag, int default_value) {
  const std::string env_var = FlagToEnvVar(flag);
  const char* const value_str = getenv(env_var.c_str());
  int value = default_value;
  if (value_str == nullptr ||
      !ParseInt(std::string("Env variable ") + env_var, value_str, &value))
    return default_value;
  return value;
}

// =============================================================================
bool ParseIntFlag(const char* str, const char* flag, int* value) {
  // Get value of the flag as a string.
  const char* const value_str = ParseFlagValue(str, flag);
  // Abort if the parsing failed.
  if (value_str == nullptr) return false;
  // Set `*value` to the value of the flag.
  return ParseInt(std::string("Value of flag --") + flag, value_str, value);
}

}  // namespace modcncy
