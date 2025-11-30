#pragma once
#include "crow.h"
#include <string>

class AuthController
{
public:
    static crow::response registerUser(const crow::request &req);
    static crow::response login(const crow::request &req);
    static crow::response getCurrentUser(const crow::request &req, const std::string &user_id);
};

