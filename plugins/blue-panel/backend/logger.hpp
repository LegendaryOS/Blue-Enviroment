#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <ctime>

class Logger {
public:
    Logger(const std::string& log_file = "/tmp/blue-panel.log") : log_file_(log_file) {
        ofs_.open(log_file_, std::ios::app);
    }

    ~Logger() {
        if (ofs_.is_open()) ofs_.close();
    }

    void info(const std::string& msg) {
        log("INFO", msg);
    }

    void error(const std::string& msg) {
        log("ERROR", msg);
    }

private:
    std::ofstream ofs_;
    std::string log_file_;

    void log(const std::string& level, const std::string& msg) {
        if (ofs_.is_open()) {
            time_t now = time(nullptr);
            ofs_ << "[" << ctime(&now) << "] " << level << ": " << msg << std::endl;
        }
    }
};

#endif // LOGGER_HPP
