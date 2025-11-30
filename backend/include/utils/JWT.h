#pragma once
#include <string>

class JWT
{
public:
    static std::string verifyAndGetUserId(const std::string &token);
    static bool verify(const std::string &token);
    static std::string generate(const std::string &user_id);
};

