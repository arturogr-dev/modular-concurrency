// Copyright 2022 The Modcncy Authors. All rights reserved.
// Use of this source code is governed by the license found in the LICENSE file.
// -----------------------------------------------------------------------------
//
// Utilities to handle commandline flag data.
//
// -----------------------------------------------------------------------------

#ifndef MODCNCY_INCLUDE_MODCNCY_FLAGS_H_
#define MODCNCY_INCLUDE_MODCNCY_FLAGS_H_

// Macro to reference flags.
#define FLAG(name) FLAGS_##name

// Macros to declare flags.
#define MODCNCY_DECLARE_int(name) extern int FLAG(NAME)

// Macros to define flags.
#define MODCNCY_DEFINE_int(name, default_value) \
  int FLAG(name) = modcncy::internal::IntFromEnv(#name, default_value)

namespace modcncy {
namespace internal {

// Parses an `int` from the env variable corresponding to the given flag.
// If the variable exists, returns `ParseInt()` value.
// Otherwise, return the default value.
int IntFromEnv(const char* flag, int default_value);

// Parses a `string` for an `int` flag, in the form of "--flag=value".
// On success, stores the value of the flag in `*value` and returns `true`.
// On failure, returns false without changin `*value`.
bool ParseIntFlag(const char* str, const char* flag, int* value);

}  // namespace internal
}  // namespace modcncy

#endif  // MODCNCY_INCLUDE_MODCNCY_FLAGS_H_
