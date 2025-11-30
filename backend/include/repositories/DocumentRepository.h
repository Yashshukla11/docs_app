#pragma once
#include "models/Document.h"
#include <string>
#include <optional>
#include <vector>

struct sqlite3_stmt;

class DocumentRepository
{
public:
    DocumentRepository();
    
    // CRUD operations
    std::optional<Document> createDocument(const Document& document);
    std::optional<Document> findById(const std::string& id);
    std::vector<Document> findByOwnerId(const std::string& owner_id);
    bool updateDocument(const Document& document);
    bool deleteDocument(const std::string& id);
    
    // Utility
    bool documentExists(const std::string& id);
    bool isOwner(const std::string& doc_id, const std::string& user_id);

private:
    std::string generateId();
    Document mapRowToDocument(sqlite3_stmt* stmt);
};

