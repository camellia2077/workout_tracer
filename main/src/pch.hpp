// pch.hpp
#ifndef PCH_H
#define PCH_H

// --- Standard Library Headers ---
// Core utilities
#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <format>

// I/O
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

// Algorithms and numerics
#include <numeric>
#include <limits>
#include <algorithm>

// Platform specific
#ifdef _WIN32
#include <windows.h>
#endif

// --- Third-party Libraries ---
#include "nlohmann/json.hpp"
#include "sqlite3.h"

// --- Project-specific Headers ---
// Most frequently used project headers
#include "common/parsed_data.hpp"
#include "common/JsonReader.hpp"
// Note: We include headers that are stable and widely used.
// Headers that change often should not be in the PCH.

#endif //PCH_H
