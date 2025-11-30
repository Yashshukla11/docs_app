#include "services/DocumentService.h"
#include "repositories/DocumentRepository.h"
#include "models/Document.h"
#include <stdexcept>

Document DocumentService::createDocument(const std::string& owner_id, const std::string& title, const std::string& content)
{
    // Validate input
    if (owner_id.empty())
    {
        throw std::invalid_argument("Owner ID is required");
    }
    
    if (!Document::isValidTitle(title))
    {
        throw std::invalid_argument("Title must be 1-255 characters and not empty");
    }
    
    // Create document
    Document doc("", title, content, owner_id);
    DocumentRepository repo;
    auto createdDoc = repo.createDocument(doc);
    
    if (!createdDoc.has_value())
    {
        throw std::runtime_error("Failed to create document");
    }
    
    return createdDoc.value();
}

Document DocumentService::getDocumentById(const std::string& doc_id, const std::string& user_id)
{
    if (doc_id.empty() || user_id.empty())
    {
        throw std::invalid_argument("Document ID and User ID are required");
    }
    
    DocumentRepository repo;
    auto doc = repo.findById(doc_id);
    
    if (!doc.has_value())
    {
        throw std::runtime_error("Document not found");
    }
    
    // Check ownership
    if (doc.value().getOwnerId() != user_id)
    {
        throw std::runtime_error("Access denied: You don't have permission to access this document");
    }
    
    return doc.value();
}

std::vector<Document> DocumentService::getAllUserDocuments(const std::string& user_id)
{
    if (user_id.empty())
    {
        throw std::invalid_argument("User ID is required");
    }
    
    DocumentRepository repo;
    return repo.findByOwnerId(user_id);
}

Document DocumentService::updateDocument(const std::string& doc_id, const std::string& user_id, const std::string& title, const std::string& content)
{
    if (doc_id.empty() || user_id.empty())
    {
        throw std::invalid_argument("Document ID and User ID are required");
    }
    
    if (!Document::isValidTitle(title))
    {
        throw std::invalid_argument("Title must be 1-255 characters and not empty");
    }
    
    DocumentRepository repo;
    
    // Check if document exists and user owns it
    auto doc = repo.findById(doc_id);
    if (!doc.has_value())
    {
        throw std::runtime_error("Document not found");
    }
    
    if (doc.value().getOwnerId() != user_id)
    {
        throw std::runtime_error("Access denied: You don't have permission to update this document");
    }
    
    // Update document
    Document updatedDoc = doc.value();
    updatedDoc.setTitle(title);
    updatedDoc.setContent(content);
    
    if (!repo.updateDocument(updatedDoc))
    {
        throw std::runtime_error("Failed to update document");
    }
    
    // Return updated document
    auto result = repo.findById(doc_id);
    if (!result.has_value())
    {
        throw std::runtime_error("Failed to retrieve updated document");
    }
    
    return result.value();
}

Document DocumentService::renameDocument(const std::string& doc_id, const std::string& user_id, const std::string& new_title)
{
    if (doc_id.empty() || user_id.empty())
    {
        throw std::invalid_argument("Document ID and User ID are required");
    }
    
    if (!Document::isValidTitle(new_title))
    {
        throw std::invalid_argument("Title must be 1-255 characters and not empty");
    }
    
    DocumentRepository repo;
    
    // Check if document exists and user owns it
    auto doc = repo.findById(doc_id);
    if (!doc.has_value())
    {
        throw std::runtime_error("Document not found");
    }
    
    if (doc.value().getOwnerId() != user_id)
    {
        throw std::runtime_error("Access denied: You don't have permission to rename this document");
    }
    
    // Update only title
    Document updatedDoc = doc.value();
    updatedDoc.setTitle(new_title);
    
    if (!repo.updateDocument(updatedDoc))
    {
        throw std::runtime_error("Failed to rename document");
    }
    
    // Return updated document
    auto result = repo.findById(doc_id);
    if (!result.has_value())
    {
        throw std::runtime_error("Failed to retrieve renamed document");
    }
    
    return result.value();
}

void DocumentService::deleteDocument(const std::string& doc_id, const std::string& user_id)
{
    if (doc_id.empty() || user_id.empty())
    {
        throw std::invalid_argument("Document ID and User ID are required");
    }
    
    DocumentRepository repo;
    
    // Check if document exists and user owns it
    auto doc = repo.findById(doc_id);
    if (!doc.has_value())
    {
        throw std::runtime_error("Document not found");
    }
    
    if (doc.value().getOwnerId() != user_id)
    {
        throw std::runtime_error("Access denied: You don't have permission to delete this document");
    }
    
    if (!repo.deleteDocument(doc_id))
    {
        throw std::runtime_error("Failed to delete document");
    }
}

