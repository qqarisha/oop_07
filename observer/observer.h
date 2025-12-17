#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <mutex>

class IObserver {
public:
    virtual void onKill(const std::string& killer, const std::string& victim) = 0;
    virtual void onDoubleDeath(const std::string& a, const std::string& b) = 0;
    virtual ~IObserver() = default;
};

class ConsoleObserver : public IObserver {
    mutable std::mutex cout_mutex;
public:
    void onKill(const std::string& killer, const std::string& victim) override;
    void onDoubleDeath(const std::string& a, const std::string& b) override;
};

class FileObserver : public IObserver {
    std::ofstream f;
    mutable std::mutex file_mutex;
public:
    FileObserver();
    ~FileObserver();
    void onKill(const std::string& killer, const std::string& victim) override;
    void onDoubleDeath(const std::string& a, const std::string& b) override;
};