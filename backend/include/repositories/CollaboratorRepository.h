#pragma once
#include "models/Collaborator.h"
#include <string>
#include <optional>
#include <vector>

struct sqlite3_stmt;

class CollaboratorRepository
{
public:
    CollaboratorRepository();
    
    // CRUD operations
    std::optional<Collaborator> addCollaborator(const Collaborator& collaborator);
    std::optional<Collaborator> findCollaborator(const std::string& doc_id, const std::string& user_id);
    std::vector<Collaborator> findByDocumentId(const std::string& doc_id);
    std::vector<Collaborator> findByUserId(const std::string& user_id);
    bool updatePermission(const std::string& doc_id, const std::string& user_id, const std::string& permission);
    bool removeCollaborator(const std::string& doc_id, const std::string& user_id);
    
    // Utility
    bool isCollaborator(const std::string& doc_id, const std::string& user_id);
    bool hasAccess(const std::string& doc_id, const std::string& user_id, const std::string& required_permission);

private:
    std::string generateId();
    Collaborator mapRowToCollaborator(sqlite3_stmt* stmt);
};


