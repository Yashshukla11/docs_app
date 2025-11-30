#pragma once
#include <string>

class Collaborator
{
public:
    Collaborator();
    Collaborator(const std::string& id, const std::string& document_id,
                 const std::string& user_id, const std::string& permission,
                 const std::string& shared_by);
    
    // Getters
    std::string getId() const { return id_; }
    std::string getDocumentId() const { return document_id_; }
    std::string getUserId() const { return user_id_; }
    std::string getPermission() const { return permission_; }
    std::string getSharedBy() const { return shared_by_; }
    std::string getCreatedAt() const { return created_at_; }
    std::string getUpdatedAt() const { return updated_at_; }
    
    // Setters
    void setId(const std::string& id) { id_ = id; }
    void setDocumentId(const std::string& document_id) { document_id_ = document_id; }
    void setUserId(const std::string& user_id) { user_id_ = user_id; }
    void setPermission(const std::string& permission) { permission_ = permission; }
    void setSharedBy(const std::string& shared_by) { shared_by_ = shared_by; }
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::string& updated_at) { updated_at_ = updated_at; }
    
    // Validation
    bool isValid() const;
    static bool isValidPermission(const std::string& permission);
    static bool isValidId(const std::string& id);

private:
    std::string id_;
    std::string document_id_;
    std::string user_id_;
    std::string permission_;  // "read" or "write"
    std::string shared_by_;
    std::string created_at_;
    std::string updated_at_;
};

