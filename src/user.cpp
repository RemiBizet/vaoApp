#include "user.h"

User::User(const std::string& username, const std::string& passwordHash)
    : username(username), passwordHash(passwordHash) {
    uuid_t uuid;
    char uuid_str[37];
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, uuid_str);
    userId = std::string(uuid_str);
}

User::User(const std::string& userId, const std::string& username, const std::string& passwordHash) 
    : userId(userId), username(username), passwordHash(passwordHash){}

std::string User::getUserId() const {
    return userId;
}

std::string User::getUsername() const {
    return username;
}

std::string User::getPasswordHash() const {
    return passwordHash;
}
