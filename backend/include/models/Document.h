#pragma once
#include <string>

class Document
{
public:
    Document();
    Document(const std::string &id, const std::string &title,
             const std::string &content, const std::string &owner_id);

    // Getters
    std::string getId() const { return id_; }
    std::string getTitle() const { return title_; }
    std::string getContent() const { return content_; }
    std::string getOwnerId() const { return owner_id_; }
    int getVersion() const { return version_; }
    std::string getCreatedAt() const { return created_at_; }
    std::string getUpdatedAt() const { return updated_at_; }

    // Setters
    void setId(const std::string &id) { id_ = id; }
    void setTitle(const std::string &title) { title_ = title; }
    void setContent(const std::string &content) { content_ = content; }
    void setOwnerId(const std::string &owner_id) { owner_id_ = owner_id; }
    void setVersion(int version) { version_ = version; }
    void setCreatedAt(const std::string &created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::string &updated_at) { updated_at_ = updated_at; }

    // Validation
    bool isValid() const;
    static bool isValidTitle(const std::string &title);
    static bool isValidId(const std::string &id);

private:
    std::string id_;
    std::string title_;
    std::string content_;
    std::string owner_id_;
    int version_;
    std::string created_at_;
    std::string updated_at_;
};
