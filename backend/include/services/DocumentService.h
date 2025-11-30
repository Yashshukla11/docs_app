#pragma once
#include "models/Document.h"
#include <string>
#include <vector>

class DocumentService
{
public:
    static Document createDocument(const std::string& owner_id, const std::string& title, const std::string& content = "");
    static Document getDocumentById(const std::string& doc_id, const std::string& user_id);
    static std::vector<Document> getAllUserDocuments(const std::string& user_id);
    static Document updateDocument(const std::string& doc_id, const std::string& user_id, const std::string& title, const std::string& content);
    static Document renameDocument(const std::string& doc_id, const std::string& user_id, const std::string& new_title);
    static void deleteDocument(const std::string& doc_id, const std::string& user_id);
};

