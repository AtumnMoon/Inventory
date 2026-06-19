#pragma once

/**
 * @brief ANSI escape codes for terminal text styling.
 */
#include <string>

namespace Color {
const std::string Reset = "\033[0m";
const std::string Bold = "\033[1m";
const std::string Highlight = "\033[7m";  /// @brief Reverse color

const std::string Black = "\033[30m";
const std::string Red = "\033[31m";
const std::string Green = "\033[32m";
const std::string Yellow = "\033[33m";
const std::string Blue = "\033[34m";
const std::string Magenta = "\033[35m";
const std::string Cyan = "\033[36m";
const std::string White = "\033[37m";
}  // namespace Color
