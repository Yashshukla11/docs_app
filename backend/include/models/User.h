#pragma once
#include <string>
#include <ctime>

class User
{
public:
    User();
    User(const std::string& id, const std::string& email, 
         const std::string& username, const std::string& password_hash);
    
    // Getters
    std::string getId() const { return id_; }
    std::string getEmail() const { return email_; }
    std::string getUsername() const { return username_; }
    std::string getPasswordHash() const { return password_hash_; }
    std::string getCreatedAt() const { return created_at_; }
    std::string getUpdatedAt() const { return updated_at_; }
    
    // Setters
    void setId(const std::string& id) { id_ = id; }
    void setEmail(const std::string& email) { email_ = email; }
    void setUsername(const std::string& username) { username_ = username; }
    void setPasswordHash(const std::string& password_hash) { password_hash_ = password_hash; }
    void setUpdatedAt(const std::string& updated_at) { updated_at_ = updated_at; }
    
    // Validation
    bool isValid() const;
    static bool isValidEmail(const std::string& email);
    static bool isValidUsername(const std::string& username);
    static bool isValidPassword(const std::string& password);

private:
    std::string id_;
    std::string email_;
    std::string username_;
    std::string password_hash_;
    std::string created_at_;
    std::string updated_at_;
};

