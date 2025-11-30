#include "crow.h"
#include "crow/middlewares/cors.h"
#include "routes/routes.h"
#include "db/Database.h"
#include <iostream>

int main()
{
    // Initialize database
    auto &db = Database::getInstance();
    if (!db.initialize("docs_backend.db"))
    {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }

    std::cout << "Database initialized successfully" << std::endl;

    // Enable CORS
    crow::App<crow::CORSHandler> app;
    auto &cors = app.get_middleware<crow::CORSHandler>();
    cors
        .global()
        .headers("Content-Type", "Authorization")
        .methods("GET"_method, "POST"_method, "PUT"_method, "PATCH"_method, "DELETE"_method, "OPTIONS"_method)
        .origin("*"); // Allow all origins for development/network access

    // Setup all routes
    setupRoutes(app);

    std::cout << "Server starting on port 8080..." << std::endl;
    // Start server - bind to all interfaces (0.0.0.0) for network access
    app.bindaddr("0.0.0.0").port(8080).multithreaded().run();

    // Cleanup
    db.close();
    return 0;
}
