#pragma once
#include "models/Collaborator.h"
#include "models/User.h"
#include <string>
#include <vector>

class CollaborationService
{
public:
    // Share document with user by email
    static Collaborator shareDocument(const std::string& doc_id, const std::string& owner_id, 
                                      const std::string& collaborator_email, const std::string& permission);
    
    // Get all collaborators for a document (owner or collaborator can view)
    static std::vector<Collaborator> getCollaborators(const std::string& doc_id, const std::string& user_id);
    
    // Update collaborator permission (owner only)
    static Collaborator updatePermission(const std::string& doc_id, const std::string& owner_id,
                                         const std::string& collaborator_id, const std::string& permission);
    
    // Remove collaborator (owner only)
    static void removeCollaborator(const std::string& doc_id, const std::string& owner_id,
                                   const std::string& collaborator_id);
    
    // Get all documents shared with user
    static std::vector<std::string> getSharedDocumentIds(const std::string& user_id);
    
    // Check if user has access to document with required permission
    static bool checkAccess(const std::string& doc_id, const std::string& user_id, 
                           const std::string& required_permission);
};


