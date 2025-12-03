#include "models/Collaborator.h"
#include <algorithm>

Collaborator::Collaborator() : id_(""), document_id_(""), user_id_(""), 
                               permission_(""), shared_by_(""),
                               created_at_(""), updated_at_("") {}

Collaborator::Collaborator(const std::string &id, const std::string &document_id,
                           const std::string &user_id, const std::string &permission,
                           const std::string &shared_by)
    : id_(id), document_id_(document_id), user_id_(user_id), 
      permission_(permission), shared_by_(shared_by),
      created_at_(""), updated_at_("") {}

bool Collaborator::isValid() const
{
    return !id_.empty() && !document_id_.empty() && 
           !user_id_.empty() && isValidPermission(permission_) &&
           !shared_by_.empty() && isValidId(id_) && 
           isValidId(document_id_) && isValidId(user_id_) && 
           isValidId(shared_by_);
}

bool Collaborator::isValidPermission(const std::string &permission)
{
    return permission == "read" || permission == "write";
}

bool Collaborator::isValidId(const std::string &id)
{
    // ID: non-empty, reasonable length
    return !id.empty() && id.length() <= 100;
}


