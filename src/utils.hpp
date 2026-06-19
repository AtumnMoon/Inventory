#pragma once

#include <string>
#include <vector>

/**
 * @brief Remove leading and trailing whitespace
 * @param s The string to cleanup
 */
void trim(std::string& s);

/**
 * @brief Split a string into tokens by a delimiter
 * @param s         The string to split
 * @param delimiter The delimiter to split on
 * @return The list of tokens (never empty — at least one element)
 */
std::vector<std::string> split(const std::string& s,
                               const std::string& delimiter);

/**
 * @brief A prompt helper to get a single valid char from the user
 * @param prompt The prompt to show to user
 * @return The user input
 */
char get_char(const std::string& prompt);

/**
 * @brief A prompt helper to get valid integer from the user
 * @param prompt The prompt to show to user
 * @return The user input
 */
int get_uint(const std::string& prompt);

/**
 * @brief A prompt helper to get valid integer from the user
 * @param prompt        The prompt to show to user
 * @param default_value The default value to use for skipping
 * @return The user input
 */
int get_uint_or_skip(const std::string& prompt, int default_value);

/**
 * @brief A prompt helper to get valid string from the user
 * @param prompt The prompt to show to user
 * @return The user input
 */
std::string get_string(const std::string& prompt);

/**
 * @brief A prompt helper to get valid string from the user
 * @param prompt        The prompt to show to user
 * @param default_value The default value to use for skipping
 * @return The user input
 */
std::string get_string_or_skip(const std::string& prompt,
                               const std::string& default_value);

/**
 * @brief A prompt helper to get a boolean answer from the user
 * @param prompt The prompt question to show
 */
bool prompt_yes_no(const std::string& prompt);
