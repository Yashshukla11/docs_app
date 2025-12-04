#include "routes/routes.h"
#include "controllers/AuthController.h"
#include "controllers/DocumentController.h"
#include "utils/JWT.h"
#include "utils/WebSocketManager.h"
#include "services/CollaborationService.h"
#include "services/DocumentService.h"
#include "repositories/DocumentRepository.h"
#include "repositories/UserRepository.h"
#include "models/Document.h"
#include "crow/middlewares/cors.h"
#include "crow/json.h"
#include <string>
#include <sstream>

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
    CROW_ROUTE(app, "/api/documents/<string>/collaborators/<string>")
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

    struct ConnectionData
    {
        std::string doc_id;
        std::string user_id;
    };

    CROW_WEBSOCKET_ROUTE(app, "/api/documents/ws/connect")
        .onaccept([](const crow::request &req, void **userdata)
                  {
            std::cout << "[WebSocket] Connection attempt to: " << req.url << std::endl;
            auto doc_id_param = req.url_params.get("doc_id");
            if (!doc_id_param) {
                std::cout << "[WebSocket] Missing doc_id parameter" << std::endl;
                return false;
            }
            std::string doc_id = std::string(doc_id_param);
            std::cout << "[WebSocket] Document ID: " << doc_id << std::endl;
            auto token_param = req.url_params.get("token");
            if (!token_param) {
                std::cout << "[WebSocket] Missing token parameter" << std::endl;
                return false;
            }
            std::string token = std::string(token_param);
            
            if (token.empty()) {
                std::cout << "[WebSocket] Empty token" << std::endl;
                return false;
            }
            
            std::string user_id = JWT::verifyAndGetUserId(token);
            if (user_id.empty()) {
                std::cout << "[WebSocket] Invalid token" << std::endl;
                return false;
            }
            std::cout << "[WebSocket] User ID: " << user_id << std::endl;
            bool hasAccess = CollaborationService::checkAccess(doc_id, user_id, "read");
            if (!hasAccess) {
                std::cout << "[WebSocket] Access denied for user " << user_id << " to document " << doc_id << std::endl;
                return false;
            }
            
            std::cout << "[WebSocket] Connection accepted for user " << user_id << " to document " << doc_id << std::endl;
            *userdata = new ConnectionData{doc_id, user_id};
            return true; })
        .onopen([](crow::websocket::connection &conn)
                {
            auto* data = static_cast<ConnectionData*>(conn.userdata());
            if (data) {
                std::cout << "[WebSocket] Connection opened for user " << data->user_id << " to document " << data->doc_id << std::endl;
                WebSocketManager::getInstance().joinDocument(data->doc_id, &conn, data->user_id);
                
                std::string username = "User";
                try {
                    UserRepository userRepo;
                    auto user = userRepo.findById(data->user_id);
                    if (user.has_value()) {
                        username = user.value().getUsername();
                    }
                } catch (...) {
                    // Use default username if lookup fails
                }
                
                crow::json::wvalue join_msg;
                join_msg["type"] = "user_joined";
                join_msg["user_id"] = data->user_id;
                join_msg["username"] = username;
                join_msg["doc_id"] = data->doc_id;
                std::string join_msg_str = join_msg.dump();
                WebSocketManager::getInstance().broadcastToDocument(data->doc_id, join_msg_str, &conn);
            } })
        .onclose([](crow::websocket::connection &conn, const std::string &reason, uint16_t)
                 {
            auto* data = static_cast<ConnectionData*>(conn.userdata());
            if (data) {
                crow::json::wvalue leave_msg;
                leave_msg["type"] = "user_left";
                leave_msg["user_id"] = data->user_id;
                leave_msg["doc_id"] = data->doc_id;
                std::string leave_msg_str = leave_msg.dump();
                WebSocketManager::getInstance().broadcastToDocument(data->doc_id, leave_msg_str);
                
                WebSocketManager::getInstance().leaveAll(&conn);
                delete data;
            } })
        .onmessage([](crow::websocket::connection &conn, const std::string &data, bool is_binary)
                   {
            if (is_binary) return;
            
            try {
                auto* conn_data = static_cast<ConnectionData*>(conn.userdata());
                if (!conn_data) return;
                
                auto msg = crow::json::load(data);
                if (!msg) return;
                
                std::string type = msg["type"].s();
                
                if (type == "edit") {
                    std::cout << "[WebSocket] Received edit message from user " << conn_data->user_id << " for document " << conn_data->doc_id << std::endl;
                    auto edit_msg = crow::json::load(data);
                    if (edit_msg) {
                        // Create a new message with userId included
                        crow::json::wvalue broadcast_msg;
                        broadcast_msg["type"] = "edit";
                        if (edit_msg.has("content")) {
                            broadcast_msg["content"] = std::string(edit_msg["content"].s());
                        }
                        if (edit_msg.has("version")) {
                            broadcast_msg["version"] = static_cast<int>(edit_msg["version"].i());
                        }
                        broadcast_msg["userId"] = conn_data->user_id;
                        std::string edit_msg_str = broadcast_msg.dump();
                        WebSocketManager::getInstance().broadcastToDocument(conn_data->doc_id, edit_msg_str);
                        std::cout << "[WebSocket] Broadcasted edit message to all users in document " << conn_data->doc_id << std::endl;
                    } else {
                        WebSocketManager::getInstance().broadcastToDocument(conn_data->doc_id, data, &conn);
                    }
                } else if (type == "cursor") {
                    std::string username = "User";
                    try {
                        UserRepository userRepo;
                        auto user = userRepo.findById(conn_data->user_id);
                        if (user.has_value()) {
                            username = user.value().getUsername();
                        }
                    } catch (...) {
                        // Use default username if lookup fails
                    }
                    
                    auto cursor_msg = crow::json::load(data);
                    if (cursor_msg) {
                        crow::json::wvalue cursor_wmsg;
                        cursor_wmsg["type"] = "cursor";
                        if (cursor_msg.has("position")) {
                            cursor_wmsg["position"] = static_cast<int>(cursor_msg["position"].i());
                        }
                        if (cursor_msg.has("userId")) {
                            cursor_wmsg["userId"] = std::string(cursor_msg["userId"].s());
                        }
                        cursor_wmsg["username"] = username;
                        std::string cursor_msg_str = cursor_wmsg.dump();
                        WebSocketManager::getInstance().broadcastToDocument(conn_data->doc_id, cursor_msg_str, &conn);
                    } else {
                        WebSocketManager::getInstance().broadcastToDocument(conn_data->doc_id, data, &conn);
                    }
                } else if (type == "save") {
                    if (msg.has("content")) {
                        try {
                            std::string content = msg["content"].s();
                            std::string title = msg.has("title") ? std::string(msg["title"].s()) : std::string("");
                            int expected_version = msg.has("version") ? static_cast<int>(msg["version"].i()) : -1;
                            
                            DocumentRepository docRepo;
                            auto doc = docRepo.findById(conn_data->doc_id);
                            if (doc.has_value()) {
                                std::string doc_title = title.empty() ? doc.value().getTitle() : title;
                                
                                DocumentService::updateDocument(
                                    conn_data->doc_id,
                                    conn_data->user_id,
                                    doc_title,
                                    content,
                                    expected_version
                                );
                                
                                auto updatedDoc = docRepo.findById(conn_data->doc_id);
                                if (updatedDoc.has_value()) {
                                    crow::json::wvalue save_msg;
                                    save_msg["type"] = "saved";
                                    save_msg["content"] = updatedDoc.value().getContent();
                                    save_msg["version"] = updatedDoc.value().getVersion();
                                    save_msg["userId"] = conn_data->user_id;
                                    std::string save_msg_str = save_msg.dump();
                                    
                                    WebSocketManager::getInstance().broadcastToDocument(conn_data->doc_id, save_msg_str);
                                }
                            }
                        } catch (const std::exception& e) {
                            crow::json::wvalue error_msg;
                            error_msg["type"] = "save_error";
                            error_msg["error"] = e.what();
                            std::string error_msg_str = error_msg.dump();
                            conn.send_text(error_msg_str);
                        }
                    }
                }
            } catch (const std::exception& e) {
                CROW_LOG_ERROR << "WebSocket message error: " << e.what();
            } });

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