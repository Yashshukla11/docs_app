#pragma once
#include <string>

class Crypto
{
public:
    // Password hashing
    static std::string hashPassword(const std::string& password);
    static bool verifyPassword(const std::string& password, const std::string& hash);
    
    // Generate random salt
    static std::string generateSalt();
    
private:
    // Simple SHA-256 based hashing (for production, use bcrypt or argon2)
    static std::string sha256(const std::string& input);
};

