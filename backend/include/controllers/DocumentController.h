#pragma once
#include "crow.h"
#include <string>

class DocumentController
{
public:
    // Document Management
    static crow::response getAllDocuments(const crow::request &req, const std::string &user_id);
    static crow::response createDocument(const crow::request &req, const std::string &user_id);
    static crow::response getDocument(const crow::request &req, const std::string &doc_id, const std::string &user_id);
    static crow::response updateDocument(const crow::request &req, const std::string &doc_id, const std::string &user_id);
    static crow::response renameDocument(const crow::request &req, const std::string &doc_id, const std::string &user_id);
    static crow::response deleteDocument(const crow::request &req, const std::string &doc_id, const std::string &user_id);
    
    // Collaboration & Sharing
    static crow::response shareDocument(const crow::request &req, const std::string &doc_id, const std::string &user_id);
    static crow::response getCollaborators(const crow::request &req, const std::string &doc_id, const std::string &user_id);
    static crow::response removeCollaborator(const crow::request &req, const std::string &doc_id, const std::string &collaborator_id, const std::string &user_id);
    static crow::response updatePermissions(const crow::request &req, const std::string &doc_id, const std::string &collaborator_id, const std::string &user_id);
    
    // Version History
    static crow::response getVersionHistory(const crow::request &req, const std::string &doc_id, const std::string &user_id);
    static crow::response restoreVersion(const crow::request &req, const std::string &doc_id, const std::string &version_id, const std::string &user_id);
    
    // Real-time Collaboration
    static crow::response applyOperation(const crow::request &req, const std::string &doc_id, const std::string &user_id);
    static crow::response getPendingOperations(const crow::request &req, const std::string &doc_id, const std::string &user_id);
    static void handleWebSocketMessage(crow::websocket::connection &conn, const std::string &data, const std::string &doc_id);
    
    // Comments & Suggestions
    static crow::response addComment(const crow::request &req, const std::string &doc_id, const std::string &user_id);
    static crow::response getComments(const crow::request &req, const std::string &doc_id, const std::string &user_id);
    static crow::response resolveComment(const crow::request &req, const std::string &doc_id, const std::string &comment_id, const std::string &user_id);
    
    // Search & Organization
    static crow::response searchDocuments(const crow::request &req, const std::string &user_id);
    static crow::response moveDocument(const crow::request &req, const std::string &doc_id, const std::string &user_id);
    static crow::response getRecentDocuments(const crow::request &req, const std::string &user_id);
    
    // Export
    static crow::response exportAsPDF(const crow::request &req, const std::string &doc_id, const std::string &user_id);
    static crow::response exportAsDOCX(const crow::request &req, const std::string &doc_id, const std::string &user_id);
};

