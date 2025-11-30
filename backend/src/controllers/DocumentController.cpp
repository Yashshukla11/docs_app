#include "controllers/DocumentController.h"
#include "services/DocumentService.h"
#include "models/Document.h"
#include <stdexcept>

// Document Management
crow::response DocumentController::getAllDocuments(const crow::request &req, const std::string &user_id)
{
    try
    {
        std::vector<Document> documents = DocumentService::getAllUserDocuments(user_id);

        crow::json::wvalue response;
        std::vector<crow::json::wvalue> docList;

        for (const auto &doc : documents)
        {
            crow::json::wvalue docJson;
            docJson["id"] = doc.getId();
            docJson["title"] = doc.getTitle();
            docJson["content"] = doc.getContent();
            docJson["owner_id"] = doc.getOwnerId();
            docJson["created_at"] = doc.getCreatedAt();
            docJson["updated_at"] = doc.getUpdatedAt();
            docList.push_back(docJson);
        }

        response["documents"] = crow::json::wvalue(docList);
        response["count"] = static_cast<int>(documents.size());
        return crow::response(200, response);
    }
    catch (const std::invalid_argument &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(400, response);
    }
    catch (const std::exception &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(500, response);
    }
}

crow::response DocumentController::createDocument(const crow::request &req, const std::string &user_id)
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

        std::string title = body["title"].s();
        std::string content = "";
        if (body.has("content"))
        {
            content = body["content"].s();
        }

        // Create document via service
        Document doc = DocumentService::createDocument(user_id, title, content);

        crow::json::wvalue response;
        response["message"] = "Document created successfully";
        response["document"] = {
            {"id", doc.getId()},
            {"title", doc.getTitle()},
            {"content", doc.getContent()},
            {"owner_id", doc.getOwnerId()},
            {"created_at", doc.getCreatedAt()},
            {"updated_at", doc.getUpdatedAt()}};
        return crow::response(201, response);
    }
    catch (const std::invalid_argument &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(400, response);
    }
    catch (const std::exception &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(500, response);
    }
}

crow::response DocumentController::getDocument(const crow::request &req, const std::string &doc_id, const std::string &user_id)
{
    try
    {
        Document doc = DocumentService::getDocumentById(doc_id, user_id);

        crow::json::wvalue response;
        response["document"] = {
            {"id", doc.getId()},
            {"title", doc.getTitle()},
            {"content", doc.getContent()},
            {"owner_id", doc.getOwnerId()},
            {"created_at", doc.getCreatedAt()},
            {"updated_at", doc.getUpdatedAt()}};
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
        // Check if it's access denied or not found
        std::string error_msg = e.what();
        if (error_msg.find("Access denied") != std::string::npos)
        {
            return crow::response(403, response); // Forbidden
        }
        return crow::response(404, response); // Not Found
    }
    catch (const std::exception &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(500, response);
    }
}

crow::response DocumentController::updateDocument(const crow::request &req, const std::string &doc_id, const std::string &user_id)
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

        std::string title = body["title"].s();
        std::string content = "";
        if (body.has("content"))
        {
            content = body["content"].s();
        }

        // Update document via service
        Document doc = DocumentService::updateDocument(doc_id, user_id, title, content);

        crow::json::wvalue response;
        response["message"] = "Document updated successfully";
        response["document"] = {
            {"id", doc.getId()},
            {"title", doc.getTitle()},
            {"content", doc.getContent()},
            {"owner_id", doc.getOwnerId()},
            {"created_at", doc.getCreatedAt()},
            {"updated_at", doc.getUpdatedAt()}};
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
        std::string error_msg = e.what();
        if (error_msg.find("Access denied") != std::string::npos)
        {
            return crow::response(403, response); // Forbidden
        }
        return crow::response(404, response); // Not Found
    }
    catch (const std::exception &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(500, response);
    }
}

crow::response DocumentController::renameDocument(const crow::request &req, const std::string &doc_id, const std::string &user_id)
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

        std::string new_title = body["title"].s();

        // Rename document via service
        Document doc = DocumentService::renameDocument(doc_id, user_id, new_title);

        crow::json::wvalue response;
        response["message"] = "Document renamed successfully";
        response["document"] = {
            {"id", doc.getId()},
            {"title", doc.getTitle()},
            {"content", doc.getContent()},
            {"owner_id", doc.getOwnerId()},
            {"created_at", doc.getCreatedAt()},
            {"updated_at", doc.getUpdatedAt()}};
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
        std::string error_msg = e.what();
        if (error_msg.find("Access denied") != std::string::npos)
        {
            return crow::response(403, response); // Forbidden
        }
        return crow::response(404, response); // Not Found
    }
    catch (const std::exception &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(500, response);
    }
}

crow::response DocumentController::deleteDocument(const crow::request &req, const std::string &doc_id, const std::string &user_id)
{
    try
    {
        // Delete document via service
        DocumentService::deleteDocument(doc_id, user_id);

        crow::json::wvalue response;
        response["message"] = "Document deleted successfully";
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
        std::string error_msg = e.what();
        if (error_msg.find("Access denied") != std::string::npos)
        {
            return crow::response(403, response); // Forbidden
        }
        return crow::response(404, response); // Not Found
    }
    catch (const std::exception &e)
    {
        crow::json::wvalue response;
        response["error"] = e.what();
        return crow::response(500, response);
    }
}

// Collaboration & Sharing
crow::response DocumentController::shareDocument(const crow::request &req, const std::string &doc_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["message"] = "Document shared";
    return crow::response(200, response);
}

crow::response DocumentController::getCollaborators(const crow::request &req, const std::string &doc_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["collaborators"] = crow::json::wvalue::list();
    return crow::response(200, response);
}

crow::response DocumentController::removeCollaborator(const crow::request &req, const std::string &doc_id, const std::string &collaborator_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["message"] = "Collaborator removed";
    return crow::response(200, response);
}

crow::response DocumentController::updatePermissions(const crow::request &req, const std::string &doc_id, const std::string &collaborator_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["message"] = "Permissions updated";
    return crow::response(200, response);
}

// Version History
crow::response DocumentController::getVersionHistory(const crow::request &req, const std::string &doc_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["versions"] = crow::json::wvalue::list();
    return crow::response(200, response);
}

crow::response DocumentController::restoreVersion(const crow::request &req, const std::string &doc_id, const std::string &version_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["message"] = "Version restored";
    return crow::response(200, response);
}

// Real-time Collaboration
crow::response DocumentController::applyOperation(const crow::request &req, const std::string &doc_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["message"] = "Operation applied";
    return crow::response(200, response);
}

crow::response DocumentController::getPendingOperations(const crow::request &req, const std::string &doc_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["operations"] = crow::json::wvalue::list();
    return crow::response(200, response);
}

void DocumentController::handleWebSocketMessage(crow::websocket::connection &conn, const std::string &data, const std::string &doc_id)
{
    // TODO: Implement WebSocket message handling
}

// Comments & Suggestions
crow::response DocumentController::addComment(const crow::request &req, const std::string &doc_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["message"] = "Comment added";
    return crow::response(201, response);
}

crow::response DocumentController::getComments(const crow::request &req, const std::string &doc_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["comments"] = crow::json::wvalue::list();
    return crow::response(200, response);
}

crow::response DocumentController::resolveComment(const crow::request &req, const std::string &doc_id, const std::string &comment_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["message"] = "Comment resolved";
    return crow::response(200, response);
}

// Search & Organization
crow::response DocumentController::searchDocuments(const crow::request &req, const std::string &user_id)
{
    crow::json::wvalue response;
    response["results"] = crow::json::wvalue::list();
    return crow::response(200, response);
}

crow::response DocumentController::moveDocument(const crow::request &req, const std::string &doc_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["message"] = "Document moved";
    return crow::response(200, response);
}

crow::response DocumentController::getRecentDocuments(const crow::request &req, const std::string &user_id)
{
    crow::json::wvalue response;
    response["documents"] = crow::json::wvalue::list();
    return crow::response(200, response);
}

// Export
crow::response DocumentController::exportAsPDF(const crow::request &req, const std::string &doc_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["message"] = "PDF export not implemented";
    return crow::response(501, response);
}

crow::response DocumentController::exportAsDOCX(const crow::request &req, const std::string &doc_id, const std::string &user_id)
{
    crow::json::wvalue response;
    response["message"] = "DOCX export not implemented";
    return crow::response(501, response);
}
