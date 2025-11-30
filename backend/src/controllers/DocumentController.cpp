#include "controllers/DocumentController.h"
#include "services/DocumentService.h"
#include "services/CollaborationService.h"
#include "repositories/UserRepository.h"
#include "repositories/DocumentRepository.h"
#include "models/Document.h"
#include "models/Collaborator.h"
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

        // Get version from request (optional, defaults to -1 if not provided for backward compatibility)
        int expected_version = -1;
        if (body.has("version"))
        {
            expected_version = static_cast<int>(body["version"].i());
        }

        // Update document via service
        Document doc = DocumentService::updateDocument(doc_id, user_id, title, content, expected_version);

        crow::json::wvalue response;
        response["message"] = "Document updated successfully";
        response["document"] = {
            {"id", doc.getId()},
            {"title", doc.getTitle()},
            {"content", doc.getContent()},
            {"owner_id", doc.getOwnerId()},
            {"version", doc.getVersion()},
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
        if (error_msg.find("VERSION_CONFLICT") != std::string::npos)
        {
            response["conflict"] = true;
            // Try to get current document version
            try
            {
                DocumentRepository repo;
                auto currentDoc = repo.findById(doc_id);
                if (currentDoc.has_value())
                {
                    response["current_version"] = currentDoc.value().getVersion();
                    response["current_content"] = currentDoc.value().getContent();
                }
            }
            catch (...)
            {
                // Ignore errors when fetching current document
            }
            return crow::response(409, response); // Conflict
        }
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
    try
    {
        auto body = crow::json::load(req.body);
        if (!body)
        {
            crow::json::wvalue response;
            response["error"] = "Invalid JSON";
            return crow::response(400, response);
        }

        std::string collaborator_email = body["email"].s();
        std::string permission = body["permission"].s();

        // Share document via service
        Collaborator collab = CollaborationService::shareDocument(doc_id, user_id, collaborator_email, permission);

        // Get collaborator user info
        UserRepository userRepo;
        auto collaboratorUser = userRepo.findById(collab.getUserId());

        crow::json::wvalue response;
        response["message"] = "Document shared successfully";
        response["collaboration"] = {
            {"id", collab.getId()},
            {"document_id", collab.getDocumentId()},
            {"user_id", collab.getUserId()},
            {"permission", collab.getPermission()},
            {"shared_by", collab.getSharedBy()},
            {"created_at", collab.getCreatedAt()},
            {"updated_at", collab.getUpdatedAt()}};

        if (collaboratorUser.has_value())
        {
            response["collaborator"] = {
                {"id", collaboratorUser.value().getId()},
                {"username", collaboratorUser.value().getUsername()},
                {"email", collaboratorUser.value().getEmail()}};
        }

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

crow::response DocumentController::getCollaborators(const crow::request &req, const std::string &doc_id, const std::string &user_id)
{
    try
    {
        std::vector<Collaborator> collaborators = CollaborationService::getCollaborators(doc_id, user_id);

        // Get user info for each collaborator
        UserRepository userRepo;
        std::vector<crow::json::wvalue> collabList;

        for (const auto &collab : collaborators)
        {
            crow::json::wvalue collabJson;

            // Safely set all fields with null checks
            collabJson["id"] = collab.getId().empty() ? "" : collab.getId();
            collabJson["user_id"] = collab.getUserId().empty() ? "" : collab.getUserId();
            collabJson["permission"] = collab.getPermission().empty() ? "" : collab.getPermission();
            collabJson["shared_by"] = collab.getSharedBy().empty() ? "" : collab.getSharedBy();
            collabJson["created_at"] = collab.getCreatedAt().empty() ? "" : collab.getCreatedAt();
            collabJson["updated_at"] = collab.getUpdatedAt().empty() ? "" : collab.getUpdatedAt();

            // Safely get user info
            try
            {
                if (!collab.getUserId().empty())
                {
                    auto collaboratorUser = userRepo.findById(collab.getUserId());
                    if (collaboratorUser.has_value())
                    {
                        collabJson["username"] = collaboratorUser.value().getUsername().empty() ? "" : collaboratorUser.value().getUsername();
                        collabJson["email"] = collaboratorUser.value().getEmail().empty() ? "" : collaboratorUser.value().getEmail();
                    }
                }
            }
            catch (...)
            {
                // Ignore errors when fetching user
            }

            try
            {
                if (!collab.getSharedBy().empty())
                {
                    auto sharedByUser = userRepo.findById(collab.getSharedBy());
                    if (sharedByUser.has_value())
                    {
                        collabJson["shared_by_username"] = sharedByUser.value().getUsername().empty() ? "" : sharedByUser.value().getUsername();
                    }
                }
            }
            catch (...)
            {
                // Ignore errors when fetching shared_by user
            }

            collabList.push_back(std::move(collabJson));
        }

        crow::json::wvalue response;
        response["collaborators"] = crow::json::wvalue(std::move(collabList));
        response["count"] = static_cast<int>(collaborators.size());
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

crow::response DocumentController::removeCollaborator(const crow::request &req, const std::string &doc_id, const std::string &collaborator_id, const std::string &user_id)
{
    try
    {
        // Remove collaborator via service
        CollaborationService::removeCollaborator(doc_id, user_id, collaborator_id);

        crow::json::wvalue response;
        response["message"] = "Collaborator removed successfully";
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

crow::response DocumentController::updatePermissions(const crow::request &req, const std::string &doc_id, const std::string &collaborator_id, const std::string &user_id)
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

        if (!body.has("permission"))
        {
            crow::json::wvalue response;
            response["error"] = "Permission field is required";
            return crow::response(400, response);
        }

        std::string permission = body["permission"].s();
        if (permission.empty())
        {
            crow::json::wvalue response;
            response["error"] = "Permission cannot be empty";
            return crow::response(400, response);
        }

        // Update permission via service
        Collaborator collab = CollaborationService::updatePermission(doc_id, user_id, collaborator_id, permission);

        crow::json::wvalue response;
        response["message"] = "Permission updated successfully";

        // Safely build collaboration object with null checks
        crow::json::wvalue collabJson;
        collabJson["id"] = collab.getId().empty() ? "" : collab.getId();
        collabJson["document_id"] = collab.getDocumentId().empty() ? "" : collab.getDocumentId();
        collabJson["user_id"] = collab.getUserId().empty() ? "" : collab.getUserId();
        collabJson["permission"] = collab.getPermission().empty() ? "" : collab.getPermission();
        collabJson["updated_at"] = collab.getUpdatedAt().empty() ? "" : collab.getUpdatedAt();
        response["collaboration"] = std::move(collabJson);

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
