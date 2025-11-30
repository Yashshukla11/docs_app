#include "controllers/AuthController.h"
#include "services/AuthService.h"
#include "utils/JWT.h"
#include <stdexcept>

crow::response AuthController::registerUser(const crow::request &req)
{
    try
    {
        auto body = crow::json::load(req.body);
        if (!body)
        {
            crow::json::wvalue response;
            response["error"] = "Invalid JSON";
            return crow::response(400, response);
        }

        std::string email = body["email"].s();
        std::string username = body["username"].s();
        std::string password = body["password"].s();

        // Register user via service
        User user = AuthService::registerUser(email, username, password);

        // Generate JWT token
        std::string token = JWT::generate(user.getId());

        crow::json::wvalue response;
        response["message"] = "User registered successfully";
        response["user_id"] = user.getId();
        response["email"] = user.getEmail();
        response["username"] = user.getUsername();
        response["token"] = token;
        return crow::response(201, response);
    }
    catch (const std::invalid_argument &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(400, response);
    }
    catch (const std::runtime_error &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(409, response); // Conflict
    }
    catch (const std::exception &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(500, response);
    }
}

crow::response AuthController::login(const crow::request &req)
{
    try
    {
        auto body = crow::json::load(req.body);
        if (!body)
        {
            crow::json::wvalue response;
            response["error"] = "Invalid JSON";
            return crow::response(400, response);
        }

        std::string email = body["email"].s();
        std::string password = body["password"].s();

        // Login via service
        User user = AuthService::login(email, password);

        // Generate JWT token
        std::string token = JWT::generate(user.getId());

        crow::json::wvalue response;
        response["message"] = "Login successful";
        response["user_id"] = user.getId();
        response["email"] = user.getEmail();
        response["username"] = user.getUsername();
        response["token"] = token;
        return crow::response(200, response);
    }
    catch (const std::invalid_argument &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(400, response);
    }
    catch (const std::runtime_error &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(401, response); // Unauthorized
    }
    catch (const std::exception &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(500, response);
    }
}

crow::response AuthController::getCurrentUser(const crow::request &req, const std::string &user_id)
{
    try
    {
        // Get user via service
        User user = AuthService::getUserById(user_id);

        crow::json::wvalue response;
        response["user_id"] = user.getId();
        response["email"] = user.getEmail();
        response["username"] = user.getUsername();
        return crow::response(200, response);
    }
    catch (const std::invalid_argument &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(400, response);
    }
    catch (const std::runtime_error &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(404, response); // Not Found
    }
    catch (const std::exception &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(500, response);
    }
}
