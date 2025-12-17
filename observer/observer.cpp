#include "observer.h"
#include <chrono>
#include <iomanip>

ConsoleObserver console_obs;

void ConsoleObserver::onKill(const std::string& killer, const std::string& victim) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::cout << std::put_time(std::localtime(&time), "[%H:%M:%S] ")
              << killer << " убил " << victim << std::endl;
}

void ConsoleObserver::onDoubleDeath(const std::string& a, const std::string& b) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::cout << std::put_time(std::localtime(&time), "[%H:%M:%S] ")
              << a << " и " << b << " погибли вместе" << std::endl;
}

FileObserver::FileObserver() : f("log.txt", std::ios::app) {
    if (f.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        f << "\nНАЧАЛО СЕССИИ: " << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << std::endl;
    }
}

FileObserver::~FileObserver() {
    if (f.is_open()) {
        f.close();
    }
}

void FileObserver::onKill(const std::string& killer, const std::string& victim) {
    std::lock_guard<std::mutex> lock(file_mutex);
    if (f.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        f << std::put_time(std::localtime(&time), "[%H:%M:%S] ")
          << killer << " убил " << victim << std::endl;
    }
}

void FileObserver::onDoubleDeath(const std::string& a, const std::string& b) {
    std::lock_guard<std::mutex> lock(file_mutex);
    if (f.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        f << std::put_time(std::localtime(&time), "[%H:%M:%S] ")
          << a << " и " << b << " погибли вместе" << std::endl;
    }
}