#include "routes/routes.h"
#include "controllers/AuthController.h"
#include "controllers/DocumentController.h"
#include "utils/JWT.h"
#include "crow/middlewares/cors.h"
#include <string>

// Middleware to verify JWT token and extract user info
std::pair<bool, std::string> verifyAndExtractUser(const crow::request &req)
{
    auto auth_header = req.get_header_value("Authorization");

    if (auth_header.empty())
    {
        return {false, ""};
    }

    std::string token;
    if (auth_header.find("Bearer ") == 0)
    {
        token = auth_header.substr(7);
    }
    else
    {
        return {false, ""};
    }

    std::string user_id = JWT::verifyAndGetUserId(token);
    if (user_id.empty())
    {
        return {false, ""};
    }

    return {true, user_id};
}

void setupRoutes(crow::App<crow::CORSHandler> &app)
{

    // ==================== HEALTH & STATUS ====================
    CROW_ROUTE(app, "/health")
        .methods("GET"_method)([]()
                               {
        crow::json::wvalue response;
        response["status"] = "ok";
        response["service"] = "docs-backend";
        return crow::response(200, response); });

    // ==================== AUTH ROUTES ====================

    // User registration
    CROW_ROUTE(app, "/api/auth/register")
        .methods("POST"_method)([](const crow::request &req)
                                {
        try {
            return AuthController::registerUser(req);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // User login
    CROW_ROUTE(app, "/api/auth/login")
        .methods("POST"_method)([](const crow::request &req)
                                {
        try {
            return AuthController::login(req);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Get current user profile
    CROW_ROUTE(app, "/api/auth/me")
        .methods("GET"_method)([](const crow::request &req)
                               {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return AuthController::getCurrentUser(req, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Logout
    CROW_ROUTE(app, "/api/auth/logout")
        .methods("POST"_method)([](const crow::request &req)
                                {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        crow::json::wvalue response;
        response["message"] = "Logged out successfully";
        return crow::response(200, response); });

    // ==================== DOCUMENT MANAGEMENT ====================

    // Get all documents for user (including shared with them)
    CROW_ROUTE(app, "/api/documents")
        .methods("GET"_method)([](const crow::request &req)
                               {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::getAllDocuments(req, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Create new document
    CROW_ROUTE(app, "/api/documents")
        .methods("POST"_method)([](const crow::request &req)
                                {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::createDocument(req, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Get single document by ID
    CROW_ROUTE(app, "/api/documents/<string>")
        .methods("GET"_method)([](const crow::request &req, std::string doc_id)
                               {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::getDocument(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Update document content (auto-save)
    CROW_ROUTE(app, "/api/documents/<string>")
        .methods("PATCH"_method)([](const crow::request &req, std::string doc_id)
                                 {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::updateDocument(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Rename document
    CROW_ROUTE(app, "/api/documents/<string>/rename")
        .methods("PATCH"_method)([](const crow::request &req, std::string doc_id)
                                 {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::renameDocument(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Delete document (move to trash)
    CROW_ROUTE(app, "/api/documents/<string>")
        .methods("DELETE"_method)([](const crow::request &req, std::string doc_id)
                                  {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::deleteDocument(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // ==================== COLLABORATION & SHARING ====================

    // Share document with other users
    CROW_ROUTE(app, "/api/documents/<string>/share")
        .methods("POST"_method)([](const crow::request &req, std::string doc_id)
                                {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::shareDocument(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Get list of collaborators
    CROW_ROUTE(app, "/api/documents/<string>/collaborators")
        .methods("GET"_method)([](const crow::request &req, std::string doc_id)
                               {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::getCollaborators(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Remove collaborator
    CROW_ROUTE(app, "/api/documents/<string>/collaborators/<string>")
        .methods("DELETE"_method)([](const crow::request &req, std::string doc_id, std::string collaborator_id)
                                  {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::removeCollaborator(req, doc_id, collaborator_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Update collaborator permissions
    CROW_ROUTE(app, "/api/documents/<string>/collaborators/<string>/permissions")
        .methods("PATCH"_method)([](const crow::request &req, std::string doc_id, std::string collaborator_id)
                                 {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::updatePermissions(req, doc_id, collaborator_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // ==================== VERSION HISTORY ====================

    // Get version history of document
    CROW_ROUTE(app, "/api/documents/<string>/versions")
        .methods("GET"_method)([](const crow::request &req, std::string doc_id)
                               {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::getVersionHistory(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Restore specific version
    CROW_ROUTE(app, "/api/documents/<string>/versions/<string>/restore")
        .methods("POST"_method)([](const crow::request &req, std::string doc_id, std::string version_id)
                                {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::restoreVersion(req, doc_id, version_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // ==================== REAL-TIME COLLABORATION ====================

    // Apply operational transform (for real-time editing)
    CROW_ROUTE(app, "/api/documents/<string>/operations")
        .methods("POST"_method)([](const crow::request &req, std::string doc_id)
                                {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::applyOperation(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Get pending operations since last sync
    CROW_ROUTE(app, "/api/documents/<string>/operations")
        .methods("GET"_method)([](const crow::request &req, std::string doc_id)
                               {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::getPendingOperations(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // WebSocket endpoint for real-time collaboration
    // TODO: Fix WebSocket route with CORS middleware
    // Note: WebSocket handlers don't support route parameters directly
    // The doc_id would need to be extracted from the connection URL or stored in userdata
    /*
    CROW_WEBSOCKET_ROUTE(app, "/api/documents/<string>/ws")
        .onopen([](crow::websocket::connection &conn)
                {
        // Handle WebSocket connection open
        // Extract doc_id from conn.get_url() if needed
        CROW_LOG_INFO << "WebSocket opened"; })
        .onclose([](crow::websocket::connection &conn, const std::string &reason, uint16_t)
                 {
        // Handle WebSocket connection close
        CROW_LOG_INFO << "WebSocket closed: " << reason; })
        .onmessage([](crow::websocket::connection &conn, const std::string &data, bool is_binary)
                   {
        // Handle incoming WebSocket messages (real-time edits)
        try {
            // Extract doc_id from connection URL if needed
            // For now, just log the message
            CROW_LOG_INFO << "WebSocket message received: " << data;
        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "WebSocket error: " << e.what();
        } });
    */

    // ==================== COMMENTS & SUGGESTIONS ====================

    // Add comment to document
    CROW_ROUTE(app, "/api/documents/<string>/comments")
        .methods("POST"_method)([](const crow::request &req, std::string doc_id)
                                {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::addComment(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Get all comments on document
    CROW_ROUTE(app, "/api/documents/<string>/comments")
        .methods("GET"_method)([](const crow::request &req, std::string doc_id)
                               {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::getComments(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Resolve comment
    CROW_ROUTE(app, "/api/documents/<string>/comments/<string>/resolve")
        .methods("PATCH"_method)([](const crow::request &req, std::string doc_id, std::string comment_id)
                                 {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::resolveComment(req, doc_id, comment_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // ==================== SEARCH & ORGANIZATION ====================

    // Search documents
    CROW_ROUTE(app, "/api/documents/search")
        .methods("GET"_method)([](const crow::request &req)
                               {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::searchDocuments(req, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Move document to folder
    CROW_ROUTE(app, "/api/documents/<string>/move")
        .methods("PATCH"_method)([](const crow::request &req, std::string doc_id)
                                 {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::moveDocument(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Get recently accessed documents
    CROW_ROUTE(app, "/api/documents/recent")
        .methods("GET"_method)([](const crow::request &req)
                               {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::getRecentDocuments(req, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // ==================== EXPORT ====================

    // Export document as PDF
    CROW_ROUTE(app, "/api/documents/<string>/export/pdf")
        .methods("GET"_method)([](const crow::request &req, std::string doc_id)
                               {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::exportAsPDF(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // Export document as DOCX
    CROW_ROUTE(app, "/api/documents/<string>/export/docx")
        .methods("GET"_method)([](const crow::request &req, std::string doc_id)
                               {
        auto [valid, user_id] = verifyAndExtractUser(req);
        if (!valid) {
            return crow::response(401, "{\"error\":\"Unauthorized\"}");
        }
        
        try {
            return DocumentController::exportAsDOCX(req, doc_id, user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["error"] = e.what();
            return crow::response(500, response);
        } });

    // ==================== 404 HANDLER ====================

    CROW_CATCHALL_ROUTE(app)
    ([](crow::response &res)
     {
        crow::json::wvalue response;
        response["error"] = "Route not found";
        res.code = 404;
        res.write(response.dump());
        res.end(); });
}