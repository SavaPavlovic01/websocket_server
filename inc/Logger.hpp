#pragma once 
#include <iostream>
#include <ostream>
#include <optional>
#include <string>
#include <memory>
#include <fstream>
#include <chrono>
#include <ctime>

#define RESET_COLOR "\033[0m"

enum class LogLevel{
    INFO,
    ERROR,
    WARNING
};

class Logger{
public:
    
    Logger(const std::string& filename): outFile(std::make_optional<std::ofstream>(filename, std::ofstream::app)) {}

    Logger():outFile(std::nullopt){}

    void logError(const std::string& msg){
        if(outFile.has_value()){
            log(msg, LogLevel::ERROR, outFile.value());
        } else log(msg, LogLevel::ERROR, std::cout);
    }

    void logInfo(const std::string& msg){
        if(outFile.has_value()){
            log(msg, LogLevel::INFO, outFile.value());
        } else log(msg, LogLevel::INFO, std::cout);
    }

    void logWarning(const std::string& msg){
        if(outFile.has_value()){
            log(msg, LogLevel::WARNING, outFile.value());
        } else log(msg, LogLevel::WARNING, std::cout);
    }

private:

    void log(const std::string &msg, const LogLevel level, std::ostream& out){
        out<<levelToColor(level);
        out<<"[ "<<getTimestamp()<< " ] ";
        out<<levelToString(level)<<": ";
        out<<msg<<std::endl;
        out<< RESET_COLOR;
    }

    constexpr std::string levelToString(LogLevel level){
        switch (level){
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

    constexpr std::string levelToColor(LogLevel level){
        switch (level){
        case LogLevel::INFO:
            return "\033[32m";;
        case LogLevel::WARNING:
            return "\033[33m";
        case LogLevel::ERROR:
            return "\033[31m";
        default:
            return "";
        }
    }

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm = *std::localtime(&now_c);

        std::ostringstream oss;
        oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::optional<std::ofstream> outFile;
};