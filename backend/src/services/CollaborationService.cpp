#include "services/CollaborationService.h"
#include "repositories/CollaboratorRepository.h"
#include "repositories/DocumentRepository.h"
#include "repositories/UserRepository.h"
#include "models/Collaborator.h"
#include "models/User.h"
#include <stdexcept>

Collaborator CollaborationService::shareDocument(const std::string& doc_id, const std::string& owner_id,
                                                  const std::string& collaborator_email, const std::string& permission)
{
    // Validate input
    if (doc_id.empty() || owner_id.empty() || collaborator_email.empty())
    {
        throw std::invalid_argument("Document ID, owner ID, and collaborator email are required");
    }
    
    if (!Collaborator::isValidPermission(permission))
    {
        throw std::invalid_argument("Permission must be 'read' or 'write'");
    }
    
    // Validate document exists and user is owner
    DocumentRepository docRepo;
    auto doc = docRepo.findById(doc_id);
    if (!doc.has_value())
    {
        throw std::runtime_error("Document not found");
    }
    
    if (doc.value().getOwnerId() != owner_id)
    {
        throw std::runtime_error("Access denied: Only document owner can share");
    }
    
    // Find collaborator user by email
    UserRepository userRepo;
    auto collaboratorUser = userRepo.findByEmail(collaborator_email);
    if (!collaboratorUser.has_value())
    {
        throw std::runtime_error("User not found with email: " + collaborator_email);
    }
    
    std::string collaborator_id = collaboratorUser.value().getId();
    
    // Prevent self-sharing
    if (collaborator_id == owner_id)
    {
        throw std::invalid_argument("Cannot share document with yourself");
    }
    
    // Check if already a collaborator
    CollaboratorRepository collabRepo;
    auto existing = collabRepo.findCollaborator(doc_id, collaborator_id);
    
    if (existing.has_value())
    {
        // Update existing collaboration
        if (!collabRepo.updatePermission(doc_id, collaborator_id, permission))
        {
            throw std::runtime_error("Failed to update collaboration");
        }
        
        auto updated = collabRepo.findCollaborator(doc_id, collaborator_id);
        if (!updated.has_value())
        {
            throw std::runtime_error("Failed to retrieve updated collaboration");
        }
        return updated.value();
    }
    else
    {
        // Create new collaboration
        Collaborator newCollab("", doc_id, collaborator_id, permission, owner_id);
        auto created = collabRepo.addCollaborator(newCollab);
        
        if (!created.has_value())
        {
            throw std::runtime_error("Failed to create collaboration");
        }
        
        return created.value();
    }
}

std::vector<Collaborator> CollaborationService::getCollaborators(const std::string& doc_id, const std::string& user_id)
{
    if (doc_id.empty() || user_id.empty())
    {
        throw std::invalid_argument("Document ID and User ID are required");
    }
    
    // Check if user is owner or collaborator
    DocumentRepository docRepo;
    auto doc = docRepo.findById(doc_id);
    if (!doc.has_value())
    {
        throw std::runtime_error("Document not found");
    }
    
    bool isOwner = doc.value().getOwnerId() == user_id;
    bool isCollaborator = false;
    
    if (!isOwner)
    {
        CollaboratorRepository collabRepo;
        isCollaborator = collabRepo.isCollaborator(doc_id, user_id);
    }
    
    if (!isOwner && !isCollaborator)
    {
        throw std::runtime_error("Access denied: You don't have permission to view collaborators");
    }
    
    // Get all collaborators
    CollaboratorRepository collabRepo;
    return collabRepo.findByDocumentId(doc_id);
}

Collaborator CollaborationService::updatePermission(const std::string& doc_id, const std::string& owner_id,
                                                     const std::string& collaborator_id, const std::string& permission)
{
    if (doc_id.empty() || owner_id.empty() || collaborator_id.empty())
    {
        throw std::invalid_argument("Document ID, owner ID, and collaborator ID are required");
    }
    
    if (!Collaborator::isValidPermission(permission))
    {
        throw std::invalid_argument("Permission must be 'read' or 'write'");
    }
    
    // Validate document exists and user is owner
    DocumentRepository docRepo;
    auto doc = docRepo.findById(doc_id);
    if (!doc.has_value())
    {
        throw std::runtime_error("Document not found");
    }
    
    if (doc.value().getOwnerId() != owner_id)
    {
        throw std::runtime_error("Access denied: Only document owner can update permissions");
    }
    
    // Check if collaboration exists
    CollaboratorRepository collabRepo;
    auto existing = collabRepo.findCollaborator(doc_id, collaborator_id);
    if (!existing.has_value())
    {
        throw std::runtime_error("Collaborator not found");
    }
    
    // Update permission
    if (!collabRepo.updatePermission(doc_id, collaborator_id, permission))
    {
        throw std::runtime_error("Failed to update permission");
    }
    
    // Return updated collaboration
    auto updated = collabRepo.findCollaborator(doc_id, collaborator_id);
    if (!updated.has_value())
    {
        throw std::runtime_error("Failed to retrieve updated collaboration");
    }
    
    return updated.value();
}

void CollaborationService::removeCollaborator(const std::string& doc_id, const std::string& owner_id,
                                               const std::string& collaborator_id)
{
    if (doc_id.empty() || owner_id.empty() || collaborator_id.empty())
    {
        throw std::invalid_argument("Document ID, owner ID, and collaborator ID are required");
    }
    
    // Validate document exists and user is owner
    DocumentRepository docRepo;
    auto doc = docRepo.findById(doc_id);
    if (!doc.has_value())
    {
        throw std::runtime_error("Document not found");
    }
    
    if (doc.value().getOwnerId() != owner_id)
    {
        throw std::runtime_error("Access denied: Only document owner can remove collaborators");
    }
    
    // Check if collaboration exists
    CollaboratorRepository collabRepo;
    auto existing = collabRepo.findCollaborator(doc_id, collaborator_id);
    if (!existing.has_value())
    {
        throw std::runtime_error("Collaborator not found");
    }
    
    // Remove collaborator
    if (!collabRepo.removeCollaborator(doc_id, collaborator_id))
    {
        throw std::runtime_error("Failed to remove collaborator");
    }
}

std::vector<std::string> CollaborationService::getSharedDocumentIds(const std::string& user_id)
{
    if (user_id.empty())
    {
        throw std::invalid_argument("User ID is required");
    }
    
    CollaboratorRepository collabRepo;
    auto collaborations = collabRepo.findByUserId(user_id);
    
    std::vector<std::string> docIds;
    for (const auto& collab : collaborations)
    {
        docIds.push_back(collab.getDocumentId());
    }
    
    return docIds;
}

bool CollaborationService::checkAccess(const std::string& doc_id, const std::string& user_id,
                                       const std::string& required_permission)
{
    if (doc_id.empty() || user_id.empty() || required_permission.empty())
    {
        return false;
    }
    
    // Check if user is owner (owners always have full access)
    DocumentRepository docRepo;
    auto doc = docRepo.findById(doc_id);
    if (doc.has_value() && doc.value().getOwnerId() == user_id)
    {
        return true;
    }
    
    // Check if user is collaborator with required permission
    CollaboratorRepository collabRepo;
    return collabRepo.hasAccess(doc_id, user_id, required_permission);
}


