#pragma once
#include "models/User.h"
#include <string>

class AuthService
{
public:
    static User registerUser(const std::string &email, const std::string &username, const std::string &password);
    static User login(const std::string &email, const std::string &password);
    static User getUserById(const std::string &user_id);
    static bool validatePassword(const std::string &password, const std::string &hash);
};

