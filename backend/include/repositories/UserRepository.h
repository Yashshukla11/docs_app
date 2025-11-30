#pragma once
#include "models/User.h"
#include <string>
#include <optional>

class UserRepository
{
public:
    UserRepository();
    
    // CRUD operations
    std::optional<User> createUser(const User& user);
    std::optional<User> findByEmail(const std::string& email);
    std::optional<User> findById(const std::string& id);
    std::optional<User> findByUsername(const std::string& username);
    bool updateUser(const User& user);
    bool deleteUser(const std::string& id);
    
    // Utility
    bool emailExists(const std::string& email);
    bool usernameExists(const std::string& username);

private:
    std::string generateId();
};

