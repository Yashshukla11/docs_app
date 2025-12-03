#pragma once
#include <string>
#include <memory>
#include <mutex>

struct sqlite3;

class Database
{
public:
    static Database &getInstance();

    bool initialize(const std::string &db_path = "docs_backend.db");
    void close();

    sqlite3 *getConnection() const { return db_; }
    std::mutex &getMutex() { return mutex_; }

    bool execute(const std::string &sql);
    bool initializeSchema();

    ~Database();

    // Delete copy constructor and assignment operator (Singleton)
    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

private:
    Database() : db_(nullptr) {}
    sqlite3 *db_;
    std::string db_path_;
    std::mutex mutex_;
};
