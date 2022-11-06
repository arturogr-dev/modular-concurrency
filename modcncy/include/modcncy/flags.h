// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Utilities to handle commandline flag data.
//
// The implementation is based on Google Benchmark Library commandline flags.
//
// -----------------------------------------------------------------------------

#ifndef MODCNCY_INCLUDE_MODCNCY_FLAGS_H_
#define MODCNCY_INCLUDE_MODCNCY_FLAGS_H_

#include <cstdint>
#include <string>

// Macro to reference flags.
#define FLAG(name) FLAGS_##name

// Macro to declare flags.
#define MODCNCY_DECLARE_int32(name) extern int32_t FLAG(name)
#define MODCNCY_DECLARE_string(name) extern std::string FLAG(name)

// Macro to define flags.
#define MODCNCY_DEFINE_int32(name, default_value) \
  int32_t FLAG(name) = modcncy::Int32FromEnv(#name, default_value)
#define MODCNCY_DEFINE_string(name, default_value) \
  std::string FLAG(name) = modcncy::StringFromEnv(#name, default_value)

namespace modcncy {

// Parses an `int` from the env variable corresponding to the given flag.
// If the variable exists, returns `ParseInt()` value.
// Otherwise, return the default value.
int Int32FromEnv(const char* flag, int32_t default_value);

// Parses a `string` for an `int` flag, in the form of "--flag=value".
// On success, stores the value of the flag in `*value` and returns `true`.
// On failure, returns false without changin `*value`.
bool ParseInt32Flag(const char* str, const char* flag, int32_t* value);

// Parses a `string` from the env variable corresponding to the given flag.
// If the variable exists, returns its value.
// Otherwise, return the default value.
const char* StringFromEnv(const char* flag, const char* default_value);

// Parses a `string` for a `string` flag, in the form of "--flag=value".
// On success, stores the value of the flag in `*value` and returns `true`.
// On failure, returns false without changin `*value`.
bool ParseStringFlag(const char* str, const char* flag, std::string* value);

}  // namespace modcncy

#endif  // MODCNCY_INCLUDE_MODCNCY_FLAGS_H_
