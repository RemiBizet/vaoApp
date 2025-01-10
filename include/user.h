#ifndef USER_H
#define USER_H

#include <string>
#include <uuid/uuid.h> // For UUID generation

class User {
private:
    std::string userId;
    std::string username;
    std::string passwordHash;

public:
    // Constructors
    User(const std::string& userId, const std::string& username, const std::string& passwordHash);
    User(const std::string& username, const std::string& passwordHash);

    // Getter for userId
    std::string getUserId() const;

    // Getter for username
    std::string getUsername() const;

    // Getter for passwordHash
    std::string getPasswordHash() const;
};

#endif // USER_H
