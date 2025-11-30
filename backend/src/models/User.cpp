#include "models/User.h"
#include <regex>
#include <algorithm>

User::User() : id_(""), email_(""), username_(""), password_hash_(""),
               created_at_(""), updated_at_("") {}

User::User(const std::string &id, const std::string &email,
           const std::string &username, const std::string &password_hash)
    : id_(id), email_(email), username_(username), password_hash_(password_hash),
      created_at_(""), updated_at_("") {}

bool User::isValid() const
{
    return !id_.empty() && isValidEmail(email_) &&
           isValidUsername(username_) && !password_hash_.empty();
}

bool User::isValidEmail(const std::string &email)
{
    if (email.empty() || email.length() > 255)
        return false;

    // Simple email regex validation
    const std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, pattern);
}

bool User::isValidUsername(const std::string &username)
{
    if (username.empty() || username.length() < 3 || username.length() > 30)
        return false;

    // Username: alphanumeric and underscore only
    return std::all_of(username.begin(), username.end(),
                       [](char c)
                       { return std::isalnum(c) || c == '_'; });
}

bool User::isValidPassword(const std::string &password)
{
    // Password: at least 8 characters
    return password.length() >= 8;
}
