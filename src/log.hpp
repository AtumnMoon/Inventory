#pragma once

#include <string>

/**
 * @brief Get the timestamp from system then convert to string
 */
std::string get_timestamp();

/**
 * @brief Create a structured log message
 * @param log_status Severity level (e.g., SUCCESS, INFO, WARN, ERROR)
 * @param component  What caused the event log?
 * @param message    Human explanation for it
 */
std::string create_log(const std::string& log_status,
                       const std::string& component,
                       const std::string& message);

/**
 * @brief Append the structured log message to the .log file, one event per line
 * @param log_message The structured log message
 */
void save_log(const std::string& log_message);
