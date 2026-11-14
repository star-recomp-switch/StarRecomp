// KirbyRecomp - Main public header
//
// This header provides the public API for KirbyRecomp, a static recompiler
// for Nintendo Switch (AArch64) binaries targeting the Kirby franchise.
//
// Include this header to access all KirbyRecomp functionality.

#pragma once

// Common types
#include "kirbyrecomp/types.h"

// Core numeric operations (bit-exact AArch64 semantics)
#include "kirbyrecomp/numeric.h"

// Runtime types and functions
#include "kirbyrecomp/runtime.h"

// Codegen backend
#include "kirbyrecomp/codegen/codegen.h"

namespace kr {

// Version information
constexpr const char* version_string() { return "0.1.0-milestone1"; }
constexpr int version_major() { return 0; }
constexpr int version_minor() { return 1; }
constexpr int version_patch() { return 0; }

// Re-export numeric namespace for convenience
using namespace sr::numeric;

} // namespace kr