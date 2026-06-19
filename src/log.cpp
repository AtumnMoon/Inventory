#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

#include "log.hpp"

std::string get_timestamp() {
    // Drop anything finer than seconds so the log stays readable
    const auto now =
        std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    return std::format("{:%Y-%m-%d %H:%M:%S}", now);
}

std::string create_log(const std::string& log_status,
                       const std::string& component,
                       const std::string& message) {
    // Format:
    //    2026-06-18 21:17:05 — [ STATUS ] component ||| message
    std::string log;
    log = get_timestamp();
    log += " — ";  // Separator
    log += log_status;
    log += " ";
    log += component;
    log += " ||| ";  // Separator
    log += message;
    return log;
}

void save_log(const std::string& log_message) {
    // Open log file
    const std::string file = "inventory.log";
    std::ofstream log_file(file, std::ios::app);

    // Check if its open
    if (!log_file.is_open()) {
        std::cerr << "[ ERROR ] Failed to open \"" << file << "\"\n";
        return;
    }

    // Add the log message into it, one event per line
    log_file << log_message << '\n';

    // Close the file stream
    log_file.close();
}
