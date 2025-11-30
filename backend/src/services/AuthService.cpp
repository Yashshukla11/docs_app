#include "services/AuthService.h"
#include "repositories/UserRepository.h"
#include "utils/Crypto.h"
#include "utils/JWT.h"
#include "models/User.h"
#include <stdexcept>

User AuthService::registerUser(const std::string& email, const std::string& username, const std::string& password)
{
    // Validate input
    if (!User::isValidEmail(email))
    {
        throw std::invalid_argument("Invalid email format");
    }
    
    if (!User::isValidUsername(username))
    {
        throw std::invalid_argument("Username must be 3-30 characters and alphanumeric/underscore only");
    }
    
    if (!User::isValidPassword(password))
    {
        throw std::invalid_argument("Password must be at least 8 characters");
    }
    
    // Check if user already exists
    UserRepository repo;
    if (repo.emailExists(email))
    {
        throw std::runtime_error("Email already registered");
    }
    
    if (repo.usernameExists(username))
    {
        throw std::runtime_error("Username already taken");
    }
    
    // Hash password
    std::string password_hash = Crypto::hashPassword(password);
    
    // Create user
    User user("", email, username, password_hash);
    auto createdUser = repo.createUser(user);
    
    if (!createdUser.has_value())
    {
        throw std::runtime_error("Failed to create user");
    }
    
    return createdUser.value();
}

User AuthService::login(const std::string& email, const std::string& password)
{
    if (email.empty() || password.empty())
    {
        throw std::invalid_argument("Email and password are required");
    }
    
    UserRepository repo;
    auto userOpt = repo.findByEmail(email);
    
    if (!userOpt.has_value())
    {
        throw std::runtime_error("Invalid email or password");
    }
    
    User user = userOpt.value();
    
    // Verify password
    if (!Crypto::verifyPassword(password, user.getPasswordHash()))
    {
        throw std::runtime_error("Invalid email or password");
    }
    
    return user;
}

User AuthService::getUserById(const std::string& user_id)
{
    if (user_id.empty())
    {
        throw std::invalid_argument("User ID is required");
    }
    
    UserRepository repo;
    auto userOpt = repo.findById(user_id);
    
    if (!userOpt.has_value())
    {
        throw std::runtime_error("User not found");
    }
    
    return userOpt.value();
}

bool AuthService::validatePassword(const std::string& password, const std::string& hash)
{
    return Crypto::verifyPassword(password, hash);
}

