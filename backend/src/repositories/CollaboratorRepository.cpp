#include "repositories/CollaboratorRepository.h"
#include "db/Database.h"
#include <sqlite3.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <iostream>

CollaboratorRepository::CollaboratorRepository() {}

std::string CollaboratorRepository::generateId()
{
    // Generate UUID-like ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::ostringstream oss;
    oss << std::hex;
    for (int i = 0; i < 32; ++i)
    {
        if (i == 8 || i == 12 || i == 16 || i == 20)
            oss << "-";
        oss << dis(gen);
    }
    return oss.str();
}

Collaborator CollaboratorRepository::mapRowToCollaborator(sqlite3_stmt* stmt)
{
    Collaborator collab;
    const char* id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    const char* document_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    const char* user_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    const char* permission = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    const char* shared_by = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    const char* created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    const char* updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    
    collab.setId(id ? id : "");
    collab.setDocumentId(document_id ? document_id : "");
    collab.setUserId(user_id ? user_id : "");
    collab.setPermission(permission ? permission : "");
    collab.setSharedBy(shared_by ? shared_by : "");
    collab.setCreatedAt(created_at ? created_at : "");
    collab.setUpdatedAt(updated_at ? updated_at : "");
    
    return collab;
}

std::optional<Collaborator> CollaboratorRepository::addCollaborator(const Collaborator& collaborator)
{
    auto& db = Database::getInstance();
    sqlite3* conn = db.getConnection();
    
    if (!conn)
        return std::nullopt;
    
    std::string id = collaborator.getId().empty() ? generateId() : collaborator.getId();
    Collaborator newCollab = collaborator;
    newCollab.setId(id);
    
    const char* sql = R"(
        INSERT INTO document_collaborators (id, document_id, user_id, permission, shared_by, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, datetime('now'), datetime('now'))
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return std::nullopt;
    }
    
    std::string id_str = newCollab.getId();
    std::string doc_id_str = newCollab.getDocumentId();
    std::string user_id_str = newCollab.getUserId();
    std::string permission_str = newCollab.getPermission();
    std::string shared_by_str = newCollab.getSharedBy();
    
    sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, doc_id_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user_id_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, permission_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, shared_by_str.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return std::nullopt;
    }
    
    // Fetch the created collaborator with timestamps
    return findCollaborator(doc_id_str, user_id_str);
}

std::optional<Collaborator> CollaboratorRepository::findCollaborator(const std::string& doc_id, const std::string& user_id)
{
    auto& db = Database::getInstance();
    sqlite3* conn = db.getConnection();
    
    if (!conn)
        return std::nullopt;
    
    const char* sql = "SELECT id, document_id, user_id, permission, shared_by, created_at, updated_at FROM document_collaborators WHERE document_id = ? AND user_id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK)
        return std::nullopt;
    
    sqlite3_bind_text(stmt, 1, doc_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user_id.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW)
    {
        Collaborator collab = mapRowToCollaborator(stmt);
        sqlite3_finalize(stmt);
        return collab;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Collaborator> CollaboratorRepository::findByDocumentId(const std::string& doc_id)
{
    std::vector<Collaborator> collaborators;
    auto& db = Database::getInstance();
    sqlite3* conn = db.getConnection();
    
    if (!conn)
        return collaborators;
    
    const char* sql = "SELECT id, document_id, user_id, permission, shared_by, created_at, updated_at FROM document_collaborators WHERE document_id = ? ORDER BY created_at ASC";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK)
        return collaborators;
    
    sqlite3_bind_text(stmt, 1, doc_id.c_str(), -1, SQLITE_TRANSIENT);
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        collaborators.push_back(mapRowToCollaborator(stmt));
    }
    
    sqlite3_finalize(stmt);
    return collaborators;
}

std::vector<Collaborator> CollaboratorRepository::findByUserId(const std::string& user_id)
{
    std::vector<Collaborator> collaborators;
    auto& db = Database::getInstance();
    sqlite3* conn = db.getConnection();
    
    if (!conn)
        return collaborators;
    
    const char* sql = "SELECT id, document_id, user_id, permission, shared_by, created_at, updated_at FROM document_collaborators WHERE user_id = ? ORDER BY created_at DESC";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK)
        return collaborators;
    
    sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_TRANSIENT);
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        collaborators.push_back(mapRowToCollaborator(stmt));
    }
    
    sqlite3_finalize(stmt);
    return collaborators;
}

bool CollaboratorRepository::updatePermission(const std::string& doc_id, const std::string& user_id, const std::string& permission)
{
    auto& db = Database::getInstance();
    sqlite3* conn = db.getConnection();
    
    if (!conn)
        return false;
    
    const char* sql = R"(
        UPDATE document_collaborators 
        SET permission = ?, updated_at = datetime('now')
        WHERE document_id = ? AND user_id = ?
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, permission.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, doc_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user_id.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return false;
    }
    
    return true;
}

bool CollaboratorRepository::removeCollaborator(const std::string& doc_id, const std::string& user_id)
{
    auto& db = Database::getInstance();
    sqlite3* conn = db.getConnection();
    
    if (!conn)
        return false;
    
    const char* sql = "DELETE FROM document_collaborators WHERE document_id = ? AND user_id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, doc_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user_id.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(conn) << std::endl;
        return false;
    }
    
    return true;
}

bool CollaboratorRepository::isCollaborator(const std::string& doc_id, const std::string& user_id)
{
    return findCollaborator(doc_id, user_id).has_value();
}

bool CollaboratorRepository::hasAccess(const std::string& doc_id, const std::string& user_id, const std::string& required_permission)
{
    auto collab = findCollaborator(doc_id, user_id);
    if (!collab.has_value())
        return false;
    
    // If required permission is 'read', both 'read' and 'write' have access
    if (required_permission == "read")
    {
        return collab.value().getPermission() == "read" || collab.value().getPermission() == "write";
    }
    
    // If required permission is 'write', only 'write' has access
    if (required_permission == "write")
    {
        return collab.value().getPermission() == "write";
    }
    
    return false;
}

