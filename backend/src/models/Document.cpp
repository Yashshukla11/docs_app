#include "models/Document.h"
#include <algorithm>

Document::Document() : id_(""), title_(""), content_(""), owner_id_(""),
                       created_at_(""), updated_at_("") {}

Document::Document(const std::string &id, const std::string &title,
                   const std::string &content, const std::string &owner_id)
    : id_(id), title_(title), content_(content), owner_id_(owner_id),
      created_at_(""), updated_at_("") {}

bool Document::isValid() const
{
    return !id_.empty() && isValidTitle(title_) &&
           !owner_id_.empty() && isValidId(id_) && isValidId(owner_id_);
}

bool Document::isValidTitle(const std::string &title)
{
    // Title: 1-255 characters, not just whitespace
    if (title.empty() || title.length() > 255)
        return false;

    // Check if title is not just whitespace
    return std::any_of(title.begin(), title.end(),
                       [](char c)
                       { return !std::isspace(c); });
}

bool Document::isValidId(const std::string &id)
{
    // ID: non-empty, reasonable length (UUIDs are 36 chars, but we'll be flexible)
    return !id.empty() && id.length() <= 100;
}
