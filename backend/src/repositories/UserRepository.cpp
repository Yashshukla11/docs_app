#include "repositories/UserRepository.h"
#include "db/Database.h"
#include <sqlite3.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <ctime>
#include <iostream>

UserRepository::UserRepository() {}

std::string UserRepository::generateId()
{
    // Generate UUID-like ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::ostringstream oss;
    oss << std::hex;
    for (int i = 0; i < 32; ++i)
    {
        if (i == 8 || i == 12 || i == 16 || i == 20)
            oss << "-";
        oss << dis(gen);
    }
    return oss.str();
}

std::optional<User> UserRepository::createUser(const User& user)
{
    auto& db = Database::getInstance();
    std::lock_guard<std::mutex> lock(db.getMutex());
    sqlite3* conn = db.getConnection();
    
    if (!conn)
        return std::nullopt;
    
    std::string id = user.getId().empty() ? generateId() : user.getId();
    User newUser = user;
    newUser.setId(id);
    
    const char* sql = R"(
        INSERT INTO users (id, email, username, password_hash, created_at, updated_at)
        VALUES (?, ?, ?, ?, datetime('now'), datetime('now'))
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return std::nullopt;
    }
    
    std::string id_str = newUser.getId();
    std::string email_str = newUser.getEmail();
    std::string username_str = newUser.getUsername();
    std::string password_hash_str = newUser.getPasswordHash();
    
    sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, username_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, password_hash_str.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return std::nullopt;
    }
    
    return newUser;
}

std::optional<User> UserRepository::findByEmail(const std::string& email)
{
    auto& db = Database::getInstance();
    std::lock_guard<std::mutex> lock(db.getMutex());
    sqlite3* conn = db.getConnection();
    
    if (!conn)
        return std::nullopt;
    
    const char* sql = "SELECT id, email, username, password_hash, created_at, updated_at FROM users WHERE email = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK)
        return std::nullopt;
    
    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW)
    {
        User user;
        const char* id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        
        user.setId(id ? id : "");
        user.setEmail(email ? email : "");
        user.setUsername(username ? username : "");
        user.setPasswordHash(password_hash ? password_hash : "");
        
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<User> UserRepository::findById(const std::string& id)
{
    auto& db = Database::getInstance();
    std::lock_guard<std::mutex> lock(db.getMutex());
    sqlite3* conn = db.getConnection();
    
    if (!conn)
        return std::nullopt;
    
    const char* sql = "SELECT id, email, username, password_hash, created_at, updated_at FROM users WHERE id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK)
        return std::nullopt;
    
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW)
    {
        User user;
        const char* id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        
        user.setId(id ? id : "");
        user.setEmail(email ? email : "");
        user.setUsername(username ? username : "");
        user.setPasswordHash(password_hash ? password_hash : "");
        
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<User> UserRepository::findByUsername(const std::string& username)
{
    auto& db = Database::getInstance();
    std::lock_guard<std::mutex> lock(db.getMutex());
    sqlite3* conn = db.getConnection();
    
    if (!conn)
        return std::nullopt;
    
    const char* sql = "SELECT id, email, username, password_hash, created_at, updated_at FROM users WHERE username = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK)
        return std::nullopt;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW)
    {
        User user;
        const char* id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        
        user.setId(id ? id : "");
        user.setEmail(email ? email : "");
        user.setUsername(username ? username : "");
        user.setPasswordHash(password_hash ? password_hash : "");
        
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

bool UserRepository::emailExists(const std::string& email)
{
    return findByEmail(email).has_value();
}

bool UserRepository::usernameExists(const std::string& username)
{
    return findByUsername(username).has_value();
}

bool UserRepository::updateUser(const User& user)
{
    auto& db = Database::getInstance();
    std::lock_guard<std::mutex> lock(db.getMutex());
    sqlite3* conn = db.getConnection();
    
    if (!conn)
        return false;
    
    const char* sql = R"(
        UPDATE users 
        SET email = ?, username = ?, password_hash = ?, updated_at = datetime('now')
        WHERE id = ?
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK)
        return false;
    
    std::string email_str = user.getEmail();
    std::string username_str = user.getUsername();
    std::string password_hash_str = user.getPasswordHash();
    std::string id_str = user.getId();
    
    sqlite3_bind_text(stmt, 1, email_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, username_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, password_hash_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, id_str.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool UserRepository::deleteUser(const std::string& id)
{
    auto& db = Database::getInstance();
    std::lock_guard<std::mutex> lock(db.getMutex());
    sqlite3* conn = db.getConnection();
    
    if (!conn)
        return false;
    
    const char* sql = "DELETE FROM users WHERE id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK)
        return false;
    
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

