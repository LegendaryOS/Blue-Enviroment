#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <stdexcept>

namespace utils {
    // Execute a shell command and return its output
    std::string exec(const std::string& cmd) {
        std::array<char, 128> buffer;
        std::string result;
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) throw std::runtime_error("popen() failed!");
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
        pclose(pipe);
        return result;
    }

    // Trim whitespace from string
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \n\r\t");
        size_t last = str.find_last_not_of(" \n\r\t");
        if (first == std::string::npos) return "";
        return str.substr(first, last - first + 1);
    }

    // Sanitize exec command to prevent injection
    bool sanitize_exec(const std::string& exec) {
        // Allow only alphanumeric, spaces, and common safe characters
        for (char c : exec) {
            if (!(std::isalnum(c) || c == ' ' || c == '/' || c == '-' || c == '_' || c == '.')) {
                return false;
            }
        }
        return true;
    }
}

#endif // UTILS_HPP
