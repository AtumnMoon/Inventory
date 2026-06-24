#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils.hpp"

void trim(std::string& s) {
    const std::string whitespaces = " \t\r\n";
    const std::size_t start = s.find_first_not_of(whitespaces);

    if (start == std::string::npos) {
        s.clear();
        return;
    }

    s.erase(0, start);
    const std::size_t end = s.find_last_not_of(whitespaces);
    s.erase(end + 1);
}

std::vector<std::string> split(const std::string& s,
                               const std::string& delimiter) {
    std::vector<std::string> tokens;
    std::size_t start = 0;
    std::size_t end = s.find(delimiter);

    while (end != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + delimiter.size();
        end = s.find(delimiter, start);
    }
    tokens.push_back(s.substr(start));

    return tokens;
}

char get_char(const std::string& prompt) {
    char result;

    while (true) {
        // Show the prompt
        std::cout << prompt;

        // Get the input with validation
        if (!(std::cin >> result)) {
            // Check for EOF — End of File
            // Which can be caused by Ctrl+C or Ctrl+Z
            if (std::cin.eof()) {
                // It's unrecoverable so we throw an error
                // with a meaningful message
                throw std::runtime_error(
                    "Unexpected input failure. Triggered end of file.");
            }

            // Check for Stream Failure — std::cin is down
            if (std::cin.bad()) {
                throw std::runtime_error(
                    "Unexpected input failure. Input stream failed.");
            }

            // Now we deal with recoverable error (e.g., the stream
            // was already in a bad spot before this read)
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            // Show that an error happened
            // We use `cerr` for ouputing errors
            std::cerr
                << "Invalid input. Please enter a proper letter. Try again.\n";
            continue;  // Retry
        }

        // Drop the rest of the line so the next prompt
        // (whether get_char, get_uint, or get_string) starts clean
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // Return the valid result
        return result;
    }
}

int get_uint(const std::string& prompt) {
    int result;

    while (true) {
        // Show the prompt
        std::cout << prompt;

        // Get the input with validation
        if (!(std::cin >> result)) {
            // Check for EOF — End of File
            // Which can be caused by Ctrl+C or Ctrl+Z
            if (std::cin.eof()) {
                // It's unrecoverable so we throw an error
                // with a meaningful message
                throw std::runtime_error(
                    "Unexpected input failure. Triggered end of file.");
            }

            // Check for Stream Failure — std::cin is down
            if (std::cin.bad()) {
                throw std::runtime_error(
                    "Unexpected input failure. Input stream failed.");
            }

            // Now we deal with recoverable error
            // like type mismatch (e.g., abc, ab12)
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            // Show that an error happened
            // We use `cerr` for ouputing errors
            std::cerr
                << "Invalid input. Please enter a proper number. Try again.\n";
            continue;  // Retry
        }

        // Drop the rest of the line so a later getline-based
        // prompt doesn't pick up a leftover newline
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (result < 0) {
            std::cerr << "Invalid input. Please enter a positive number. Try "
                         "again.\n";
            continue;
        }

        // Return the valid result
        return result;
    }
}

int get_uint_or_skip(const std::string& prompt, int default_value) {
    int result;

    // Show the prompt
    std::cout << prompt;

    // Get the input with validation
    if (!(std::cin >> result)) {
        // Check for EOF — End of File
        // Which can be caused by Ctrl+C or Ctrl+Z
        if (std::cin.eof()) {
            // It's unrecoverable so we throw an error
            // with a meaningful message
            throw std::runtime_error(
                "Unexpected input failure. Triggered end of file.");
        }

        // Check for Stream Failure — std::cin is down
        if (std::cin.bad()) {
            throw std::runtime_error(
                "Unexpected input failure. Input stream failed.");
        }

        // Now we deal with recoverable error
        // like type mismatch (e.g., abc, ab12)
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // Show that an error happened
        // We use `cerr` for ouputing errors
        std::cerr << "Invalid input. Not a proper number. Skipping...\n";
        return default_value;
    }

    // Drop the rest of the line for the same reason as above
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (result < 0) {
        std::cerr << "Not a positive number. Skipping...\n";
        return default_value;
    }

    // Return the valid result
    return result;
}

std::string get_string(const std::string& prompt) {
    std::string result;

    while (true) {
        // Show the prompt
        std::cout << prompt;

        // Get the input with validation
        if (!(std::getline(std::cin, result))) {
            // Check for EOF — End of File
            // Which can be caused by Ctrl+C or Ctrl+Z
            if (std::cin.eof()) {
                // It's unrecoverable so we throw an error
                // with a meaningful message
                throw std::runtime_error(
                    "Unexpected input failure. Triggered end of file.");
            }

            // Check for Stream Failure — std::cin is down
            if (std::cin.bad()) {
                throw std::runtime_error(
                    "Unexpected input failure. Input stream failed.");
            }

            // Now we deal with recoverable error
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            // Show that an error happened
            // We use `cerr` for ouputing errors
            std::cerr
                << "Invalid input. Please enter proper words. Try again.\n";
            continue;  // Retry
        }

        // Trim the whitespaces
        trim(result);

        // Check if it's empty
        if (result.empty()) {
            std::cerr
                << "Input is empty. Please enter proper words. Try again.\n";
            continue;  // Retry
        }

        // Return the valid result
        return result;
    }
}

std::string get_string_or_skip(const std::string& prompt,
                               const std::string& default_value) {
    std::string result;

    // Show the prompt
    std::cout << prompt;

    // Get the input with validation
    if (!(std::getline(std::cin, result))) {
        // Check for EOF — End of File
        // Which can be caused by Ctrl+C or Ctrl+Z
        if (std::cin.eof()) {
            // It's unrecoverable so we throw an error
            // with a meaningful message
            throw std::runtime_error(
                "Unexpected input failure. Triggered end of file.");
        }

        // Check for Stream Failure — std::cin is down
        if (std::cin.bad()) {
            throw std::runtime_error(
                "Unexpected input failure. Input stream failed.");
        }

        // Now we deal with recoverable error
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // Show that an error happened
        // We use `cerr` for ouputing errors
        std::cerr << "Invalid input. Not a proper word. Skipping...\n";
        return default_value;
    }

    // Trim the whitespaces
    trim(result);

    // Check if it's empty
    if (result.empty()) {
        std::cerr << "Input is empty. Skipping...\n";
        return default_value;
    }

    // Return the valid result
    return result;
}

bool prompt_yes_no(const std::string& prompt) {
    const std::string padding = std::string(4, ' ');
    const std::string p = padding + prompt;

    const char answer = get_char(p);

    return answer == 'y' || answer == 'Y';
}

std::string encodeCSV(const std::string& field) {
    // Check if wrapping is actually required
    if (field.find(',') == std::string::npos &&
        field.find('"') == std::string::npos &&
        field.find('\n') == std::string::npos &&
        field.find('\r') == std::string::npos) {
        return field;
    }

    std::string result = "\"";
    for (char c : field) {
        if (c == '"') {
            result += "\"\"";  // Double up inner quotes
        } else {
            result += c;
        }
    }
    result += "\"";
    return result;
}

std::string decodeCSV(const std::string& field) {
    // If it doesn't start and end with quotes, it's a raw unquoted field
    if (field.size() < 2 || field.front() != '"' || field.back() != '"') {
        return field;
    }

    std::string result = "";
    // Loop inside the outer quotes
    for (size_t i = 1; i < field.size() - 1; ++i) {
        if (field[i] == '"' && i + 1 < field.size() - 1 &&
            field[i + 1] == '"') {
            result += '"';  // Convert`""` back to `"`
            i++;            // Skip the second quote
        } else {
            result += field[i];
        }
    }
    return result;
}
